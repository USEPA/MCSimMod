#' MCSimMod class to run a model
#'
#' A class for managing MCSimMod models
#'
#' @import methods
#' @import deSolve
Model <- setRefClass("Model",
  fields = list(
    mName = "character", mString = "character", initParms = "function", initStates = "function", Outputs = "ANY",
    parms = "numeric", Y0 = "numeric", paths = "list"
  ),
  methods = list(
    initialize = function(...) {
      callSuper(...)
      if (length(mName) > 0 & length(mString) > 0) {
        stop("Cannot both have a model file `mName` and a model string `mString`")
      }
      if (length(mString) > 0) {
        file <- tempfile(pattern = "mcsimmod_", fileext = ".model")
        writeLines(mString, file)
      } else {
        file <- normalizePath(paste0(mName, ".model"))
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
      # If now checks force=T or if the hashes don't match
      hash_exists <- file.exists(paths$hash_file)
      if (hash_exists) {
        hash_has_changed <- .fileHasChanged(paths$model_file, paths$hash_file)
      } else {
        hash_has_changed <- TRUE
      }

      # Conditions for compiling a model:
      # 1. The dll file does not exist
      # 2. force = T indicating the user wants to recompile
      # 3. The hash file does not exist
      # 4. The hash file exists, but does not match the previously saved hash indicating the model file has changed
      if (!file.exists(paths$dll_file) | (force) | (!hash_exists) | (hash_exists & hash_has_changed)) {
        compileModel(paths$model_file, paths$c_file, paths$dll_name, paths$dll_file, hash_file = paths$hash_file)
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
      parms <<- initParms(new_parms)
    },
    updateY0 = function(new_states = NULL) {
      Y0 <<- initStates(parms, new_states)
    },
    runModel = function(times, method = "lsoda", ...) {
      # Solve the ODE system using the "ode" function from the package "deSolve".
      out <- ode(Y0, times,
        func = "derivs", parms = parms, dllname = paths$dll_name,
        initforc = "initforc", initfunc = "initmod", nout = length(Outputs),
        outnames = Outputs, method = method, ...
      )

      # Return the simulation output.
      return(out)
    },
    cleanup = function(deleteModel = F) {
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
