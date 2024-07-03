#' RMCSim class to run a model
#'
#' A class for managing RMCSim models
#'
#' @import methods
#' @import deSolve
#' @export Model
Model <- setRefClass("Model",
  fields = list(
    mName = "character",
    mPath = "character",
    mString = "character",
    initParms = "function",
    initStates = "function",
    Outputs = "character",
    parms = "numeric",
    Y0 = "numeric",
    paths = "list"
  ),
  methods = list(
    initialize = function(...) {
      callSuper(...)
      if (length(mName) > 0 & length(mString) > 0) {
        stop("Cannot both have a model file `mName` and a model string `mString`")
      }
      if (length(mName) > 0 & length(mPath) == 0) {
        # default to current working directory
        mPath <<- "."
      }
      if (length(mString) > 0) {
        # default to temporary directory
        if (length(mPath) == 0) {
          mPath <<- tempdir(check = T)
        }
        file <- tempfile(pattern = "mcsimmod_", tmpdir = mPath)
        mName <<- basename(file)
        writeLines(mString, paste0(file, ".model"))
      }
      mPath <<- normalizePath(mPath, mustWork = TRUE)
      paths <<- list(
        dll_name = paste0(mName, "_model"),
        dll_file = paste0(mName, "_model", .Platform$dynlib.ext),
        inits_file = paste0(mName, "_model_inits.R"),
        model_file = paste0(mName, ".model"),
        o_file = paste0(mName, "_model.o"),
        c_file = paste0(mName, "_model.c"),
        abs_dll_file = file.path(mPath, paste0(mName, "_model", .Platform$dynlib.ext)),
        abs_inits_file = file.path(mPath, paste0(mName, "_model", "_inits.R"))
      )
    },
    loadModel = function() {
      # Construct names of required files and objects from mName.
      if (!file.exists(paths$abs_dll_file)) {
        compile_model(paths$model_file, paths$c_file, paths$dll_name, paths$dll_file, mPath)
      }

      # Logic for compiling here if trying to load an uncompiled model

      # Load the compiled model (DLL).
      dyn.load(paths$abs_dll_file)

      # Run script that defines initialization functions.
      source(paths$abs_inits_file, local = TRUE)
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
      # Construct DLL name from mName.
      p0 <- getwd()
      tryCatch(
        {
          setwd(mPath)

          # Solve the ODE system using the "ode" function from the package "deSolve".
          out <- ode(Y0, times,
            func = "derivs", parms = parms, dllname = paths$dll_name,
            initforc = "initforc", initfunc = "initmod", nout = length(Outputs),
            outnames = Outputs, method = method, ...
          )
        },
        finally = {
          setwd(p0)
        }
      )

      # Return the simulation output.
      return(out)
    },
    cleanup = function() {
      # remove any model files created by compilation; unload library
      dyn.unload(paths$abs_dll_file)
      file.remove(file.path(mPath, paths$o_file))
      file.remove(file.path(mPath, paths$c_file))
      file.remove(paths$abs_inits_file)
      file.remove(paths$abs_dll_file)
    }
  )
)
