#' MCSimMod class to run a model
#' 
#' A class for managing MCSimMod models
#' 
#' @import methods
#' @import deSolve
#' @export Model
  Model <- setRefClass("Model", 
                             fields=list(mName='character', initParms='function', initStates='function', Outputs='character',
                                         parms='numeric', Y0='numeric'),
                             methods = list(
                               
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
                             }
                               )
                             )
  
  #' Return an MCSimMod instance from a string-based model
  #'
  #' Convenience method to build a string-based method.
  #'
  #' @param string model string specifying a complete MCSim ODE model
  #' @returns An MCSimMod model object
  #' @export
  fromString <- function(string){
    file <- tempfile(pattern="tmp_mcsim", tmpdir='.')
    writeLines(string, paste0(file, ".model"))
    model = Model$new(mName=basename(file))
    return(model)
  }
