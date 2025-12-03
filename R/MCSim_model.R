#' MCSimMod Model class
#'
#' A class for managing MCSimMod models.
#'
#' Instances of this class represent ordinary differential equation (ODE)
#' models. A `Model` object has both attributes (i.e., things the object “knows”
#' about itself) and methods (i.e., things the object can “do”). Model
#' attributes include: the name of the model (`mName`); a vector of parameter
#' names and values (`parms`); and a vector of initial conditions (`Y0`). Model
#' methods include functions for: translating, compiling, and loading the model
#' (`loadModel`); updating parameter values (`updateParms`); updating initial
#' conditions (`updateY0`); and running model simulations (`runModel`). So, for
#' example, if `mod` is a Model object, it will have an attribute called `parms`
#' that can be accessed using the R expression `mod$parms`. Similarly, `mod`
#' will have a method called `updateParms` that can be accessed using the R
#' expression `mod$updateParms()`. Use the `createModel()` function to create
#' `Model` objects.
#'
#' @param mName Name of an MCSim model specification file, excluding the file name extension `.model`.
#' @param mString A character string containing MCSim model specification text.
#'
#' @import methods
#' @import deSolve
Model <- setRefClass("Model",
  fields = list(
    #' @field mName Name of an MCSim model specification file, excluding the file name extension `.model`.
    #' @field mString Character string containing MCSim model specification text.
    #' @field initParms Function that initializes values of parameters defined for the associated MCSim model.
    #' @field initStates Function that initializes values of state variables defined for teh associated MCSim model..
    #' @field Outputs Names of output variables defined for the associated MCSim model.
    #' @field parms Named vector of parameter values for the associated MCSim model.
    #' @field Y0 Named vector of initial conditions for the state variables of the associated MCSim model.
    #' @field paths List of character strings that are names of files associated with the model.
    #' @field writeTemp Boolean specifying whether to write model files to a temporary directory. If value is TRUE, model files will be written to a temporary directory; if value is FALSE, model files will be written to the same directory that contains the model specification file.
    #' @field verboseOutput Boolean specifying whether to write translator messages to standard output. If value is TRUE, messages will be written to standard output; if value is FALSE, messages will be written to files in a temporary directory.
    mName = "character", mString = "character", initParms = "function",
    initStates = "function", Outputs = "ANY", parms = "numeric", Y0 = "numeric",
    paths = "list", writeTemp = "logical", verboseOutput = "logical"
  ),
  methods = list(
    initialize = function(...) {
      "Initialize the Model object using an MCSim model specification file (mName) or an MCSim model specification string (mString)."
      callSuper(...)
      if (length(mName) == 0 & length(mString) == 0) {
        stop("To create a Model object, supply either a file name (mName) or a model specification string (mString).")
      }
      if (length(mName) > 0 & length(mString) > 0) {
        stop("Cannot create a Model object using both a file name (mName) and a model specification string (mString). Provide only one of these arguments.")
      }
      # Track user's model file for proper change detection
      model_file_path <- NULL
      if (length(mString) > 0) {
        if (writeTemp == FALSE) {
          stop("The value of writeTemp must be TRUE when creating a Model object using a model specification string (mstring).")
        }
        file <- tempfile(pattern = "mcsimmod_", fileext = ".model")

        # Write model string to file with error handling and ensure it's flushed
        tryCatch(
          {
            writeLines(mString, file)
            # On Windows, ensure file is flushed to disk
            if (.Platform$OS.type == "windows") {
              Sys.sleep(0.01) # Small delay to ensure write completes
            }
            # Verify file was written correctly
            if (!file.exists(file) || file.size(file) == 0) {
              stop("Failed to write model file or file is empty: ", file)
            }
          },
          error = function(e) {
            stop("Failed to create model file from mString: ", e$message)
          }
        )

        # For mString, model and working files are the same
        model_file_path <- file
      } else {
        if (writeTemp == TRUE) {
          model_file <- normalizePath(paste0(mName, ".model"))
          model_file_path <- model_file # Store user's model file path
          temp_directory <- tempdir()
          file <- file.path(temp_directory, basename(model_file))
          file_copied <- file.copy(from = model_file, to = file)
        } else {
          file <- normalizePath(paste0(mName, ".model"))
          model_file_path <- file # writeTemp=FALSE: model and working are same
        }
      }
      mList <- .fixPath(file)
      mName <<- mList$mName
      mPath <- mList$mPath

      # Determine hash file location based on model file
      if (writeTemp == TRUE && length(mString) == 0) {
        # For writeTemp=TRUE with mName, store hash alongside user's model file
        model_mList <- .fixPath(model_file_path)
        hash_file_path <- file.path(model_mList$mPath, paste0(model_mList$mName, "_model.md5"))
      } else {
        # For writeTemp=FALSE or mString cases, store hash with working files
        hash_file_path <- file.path(mPath, paste0(mName, "_model.md5"))
      }

      paths <<- list(
        dll_name = paste0(mName, "_model"),
        c_file = file.path(mPath, paste0(mName, "_model.c")),
        o_file = file.path(mPath, paste0(mName, "_model.o")),
        dll_file = file.path(mPath, paste0(mName, "_model", .Platform$dynlib.ext)),
        inits_file = file.path(mPath, paste0(mName, "_model_inits.R")),
        source_file = file.path(mPath, paste0(mName, ".model")),
        model_file = model_file_path,
        hash_file = hash_file_path
      )
    },
    loadModel = function(force = FALSE) {
      "Translate (if necessary) the model specification text to C, compile (if necessary) the resulting C file to create a dynamic link library (DLL) file (on Windows) or a shared object (SO) file (on Unix), and then load all essential information about the Model object into memory (for use in the current R session)."
      hash_exists <- file.exists(paths$hash_file)
      if (hash_exists) {
        # Check changes against user's model file, not working copy
        hash_has_changed <- .fileHasChanged(paths$model_file, paths$hash_file)

        # If model file changed and we're using temp directory, update working copy
        if (hash_has_changed && writeTemp == TRUE && length(mString) == 0) {
          file_copied <- file.copy(from = paths$model_file, to = paths$source_file, overwrite = TRUE)
          if (!file_copied) {
            stop("Failed to update working file from model file: ", paths$model_file)
          }
        }
      } else {
        hash_has_changed <- TRUE

        # If no hash exists and we're using temp directory, ensure working copy is current
        if (writeTemp == TRUE && length(mString) == 0 && !identical(paths$model_file, paths$source_file)) {
          file_copied <- file.copy(from = paths$model_file, to = paths$source_file, overwrite = TRUE)
          if (!file_copied) {
            stop("Failed to update working file from model file: ", paths$model_file)
          }
        }
      }

      # Conditions for compiling a model:
      # 1. The DLL (on Windows) or SO (on Unix) associated with the model
      #    specification file cannot be found.
      # 2. force = TRUE, indicating the user wants to recompile.
      # 3. The hash file associated with the model specification file cannot be
      #    found
      # 4. The hash file can be found, but the contents of that file do not
      #    match the previously saved hash, indicating that the model
      #    specification file has been changed since the last translation and
      #    compiling.
      if (!file.exists(paths$dll_file) | (force) | (!hash_exists) | (hash_has_changed)) {
        # When writeTemp = TRUE and model file has changed, update the working copy
        if (writeTemp && hash_has_changed && !identical(paths$model_file, paths$source_file)) {
          file.copy(from = paths$model_file, to = paths$source_file, overwrite = TRUE)
        }

        # Call compileModel - always compile and hash the working copy (source_file)
        compileModel(paths$source_file, paths$c_file, paths$dll_name, paths$dll_file,
          hash_file = paths$hash_file, verbose_output = verboseOutput
        )
      }

      # Load the compiled model (DLL).
      dyn.load(paths$dll_file)

      # Run script that defines initialization functions.
      source(paths$inits_file, local = TRUE)

      # Associate initParms for this model with the initParms function defined
      # in inits_file.
      r_command_string <- paste0("initParms <<- initParms_", mName)
      r_expression <- parse(text = r_command_string)
      eval(r_expression)

      # Associate initStates for this model with the initStates function defined
      # in inits_file.
      r_command_string <- paste0("initStates <<- initStates_", mName)
      r_expression <- parse(text = r_command_string)
      eval(r_expression)

      # Associate Outputs for this model with the Outputs variable defined in
      # inits_file.
      r_command_string <- paste0("Outputs <<- Outputs_", mName)
      r_expression <- parse(text = r_command_string)
      eval(r_expression)

      parms <<- initParms()
      Y0 <<- initStates(parms)
    },
    updateParms = function(new_parms = NULL) {
      "Update values of parameters for the Model object."
      parms <<- initParms(new_parms)
    },
    updateY0 = function(new_states = NULL) {
      "Update values of initial conditions of state variables for the Model object."
      Y0 <<- initStates(parms, new_states)
    },
    runModel = function(times, ...) {
      "Perform a simulation for the Model object using the \\code{deSolve} function \\code{ode} for the specified \\code{times}."
      # Solve the ODE system using the "ode" function from the package "deSolve".
      derivs_name <- paste0("derivs_", mName)
      initforc_name <- paste0("initforc_", mName)
      initmod_name <- paste0("initmod_", mName)
      out <- ode(Y0, times,
        func = derivs_name, parms = parms, dllname = paths$dll_name,
        initforc = initforc_name, initfunc = initmod_name,
        nout = length(Outputs), outnames = Outputs, ...
      )

      # Return the simulation output.
      return(out)
    },
    cleanup = function(deleteModel = FALSE) {
      "Delete files created during the translation and compilation steps performed by \\code{loadModel}. If \\code{deleteModel = TRUE}, delete the MCSim model specification file, as well."
      # remove any model files created by compilation; unload library
      dyn.unload(paths$dll_file)
      if (file.exists(paths$o_file)) {
        file.remove(paths$o_file)
      }
      if (deleteModel & file.exists(paths$model_file)) {
        file.remove(paths$model_file)
      }
      if (file.exists(paths$c_file)) {
        file.remove(paths$c_file)
      }
      if (file.exists(paths$inits_file)) {
        file.remove(paths$inits_file)
      }
      if (file.exists(paths$dll_file)) {
        file.remove(paths$dll_file)
      }
      if (file.exists(paths$hash_file)) {
        file.remove(paths$hash_file)
      }
    }
  )
)
