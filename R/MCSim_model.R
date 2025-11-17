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
      if (length(mString) > 0) {
        if (writeTemp == FALSE) {
          stop("The value of writeTemp must be TRUE when creating a Model object using a model specification string (mstring).")
        }
        file <- tempfile(pattern = "mcsimmod_", fileext = ".model")
        writeLines(mString, file)
      } else {
        if (writeTemp == TRUE) {
          source_file <- normalizePath(paste0(mName, ".model"))
          temp_directory <- tempdir()
          file <- file.path(temp_directory, basename(source_file))
          file_copied <- file.copy(from = source_file, to = file)
        } else {
          file <- normalizePath(paste0(mName, ".model"))
        }
      }
      mList <- .fixPath(file)
      mName <<- mList$mName
      mPath <- mList$mPath


      paths <<- list(
        dll_name = paste0(mName, "_model"),
        c_file = file.path(mPath, paste0(mName, "_model.c")),
        o_file = file.path(mPath, paste0(mName, "_model.o")),
        dll_file = file.path(mPath, paste0(mName, "_model", .Platform$dynlib.ext)),
        inits_file = file.path(mPath, paste0(mName, "_model_inits.R")),
        model_file = file.path(mPath, paste0(mName, ".model")),
        hash_file = file.path(mPath, paste0(mName, "_model.md5"))
      )
    },
    loadModel = function(force = FALSE) {
      "Translate (if necessary) the model specification text to C, compile (if necessary) the resulting C file to create a dynamic link library (DLL) file (on Windows) or a shared object (SO) file (on Unix), and then load all essential information about the Model object into memory (for use in the current R session)."
      hash_exists <- file.exists(paths$hash_file)
      if (hash_exists) {
        hash_has_changed <- .fileHasChanged(paths$model_file, paths$hash_file)
      } else {
        hash_has_changed <- TRUE
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
      if (!file.exists(paths$dll_file) | (force) | (!hash_exists) | (hash_exists & hash_has_changed)) {
        compileModel(paths$model_file, paths$c_file, paths$dll_name, paths$dll_file, hash_file = paths$hash_file, verbose_output = verboseOutput)
      }

      # Load the compiled model (DLL).
      dyn.load(paths$dll_file)

      # Run script that defines initialization functions.
      source(paths$inits_file, local = TRUE)
      initParms <<- initParms
      initStates <<- initStates

      Outputs <<- Outputs

      parms <<- initParms()
      Y0 <<- initStates(parms)
    },
    updateParms = function(new_parms = NULL) {
      "Update values of parameters for the Model object."
      parms <<- initParms(new_parms)
    },
    updateY0 = function(new_states = NULL) {
      "Update values of initital conditions of state variables for the Model object."
      Y0 <<- initStates(parms, new_states)
    },
    runModel = function(times, ...) {
      "Perform a simulation for the Model object using the \\code{deSolve} function \\code{ode} for the specified \\code{times}."
      # Solve the ODE system using the "ode" function from the package "deSolve".
      out <- ode(Y0, times,
        func = "derivs", parms = parms, dllname = paths$dll_name,
        initforc = "initforc", initfunc = "initmod", nout = length(Outputs),
        outnames = Outputs, ...
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
