#' MCSimMod class to run a model
#' 
#' A class for managing MCSimMod models
#' 
#' @import methods
#' @import deSolve
#' @export Model
  Model <- setRefClass("Model", 
                             fields=list(mName='character', mPath = "character", mString = "character", initParms='function', initStates='function', Outputs='character',
                                         parms='numeric', Y0='numeric'),
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
                                  if (length(mPath) == 0) {
                                    # mPath <<- tempdir(check = T)
                                    mPath <<- "."
                                  }
                                  file <- tempfile(pattern = "mcsimmod_", tmpdir = mPath)
                                  mName <<- basename(file)
                                  writeLines(mString, paste0(file, ".model"))
                                }
                                mPath <<- normalizePath(mPath, mustWork = TRUE)
                                paths <<- list(
                                  dll_name = paste0(mName, "_model"),
                                  c_file = paste0(mName, "_model.c"),
                                  o_file = paste0(mName, "_model.o"),
                                  dll_file = paste0(mName, "_model", .Platform$dynlib.ext),
                                  inits_file = paste0(mName, "_model_inits.R"),
                                  model_file = paste0(mName, ".model"),
                                  abs_dll_file = file.path(mPath, paste0(mName, "_model", .Platform$dynlib.ext)),
                                  abs_inits_file = file.path(mPath, paste0(mName, "_model", "_inits.R"))
                                )
                              },
                               
                               loadModel=function() {
                                 # Construct names of required files and objects from mName.
                                 dll_name <- paste(mName, "_model", sep="")
                                 dll_file <- paste(dll_name, .Platform$dynlib.ext, sep="")
                                 inits_file <- paste(dll_name, "_inits.R", sep="")
                                 
                                 # Construct the compiled model names
                                 model_file <- paste(mName, ".model", sep="")
                                 c_file <- paste(mName, "_model.c", sep="")
                                 dll_name <- paste(mName, "_model", sep="")
                                 dll_file <- paste(dll_name, .Platform$dynlib.ext, sep="")
                                 
                                 if (!file.exists(dll_file)) {
                                   compile_model(model_file, c_file, dll_name, dll_file)
                                 }
                                 
                                 # Logic for compiling here if trying to load an uncompiled model
                                 
                                 # Load the compiled model (DLL).
                                 dyn.load(dll_file)
                                 
                                 # Run script that defines initialization functions.
                                 source(inits_file, local=TRUE)
                                 initParms <<- initParms
                                 initStates <<- initStates
                                 Outputs <<- Outputs
                                 
                                 parms <<- initParms()
                                 Y0 <<- initStates(parms)
                                 
                               
                             }, 
                             
                             updateParms = function(new_parms=NULL) {
                               parms <<- initParms(new_parms)
                               
                             },
                             
                             updateY0 = function(new_states=NULL) {
                               Y0 <<- initStates(parms,new_states)
                             },
                             
                             runModel=function(times, method="lsoda", ...) {
                                 # Construct DLL name from mName.
                                 dll_name <- paste(mName, "_model", sep="")
                                 
                                 # If parameter values are not provided, use default values.
                                 #if (is.null(parms)) {
                                 #   parms <- initParms()
                                 #}
                                 
                                 # If initial values for state variables are not provided, use default
                                 # values.
                                 #if (is.null(Y0)) {
                                 #   Y0 <- initStates(parms)
                                 #}
                                 
                                 # Solve the ODE system using the "ode" function from the package "deSolve".
                                 out <- ode(Y0, times, func="derivs", parms=parms, dllname=dll_name,
                                            initforc="initforc", initfunc="initmod", nout=length(Outputs),
                                            outnames=Outputs, method=method, ...)
                                 
                                 # Return the simulation output.
                                 return(out)
                             },

                             cleanup = function(deleteModel=F) {
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
                            }
                               )
                             )