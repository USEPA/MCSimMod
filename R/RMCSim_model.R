#' RMCSim class to run a model
#' 
#' A class for managing RMCSim models
#' 
#' @import methods
#' @import deSolve
#' @export RMCSimModel
  RMCSimModel <- setRefClass("RMCSimModel", 
                             fields=list(mName='character', initParms='function', initStates='function', Outputs='character'),
                             methods = list(
                               
                               load_model=function() {
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
                               
                             }, 
                             
                             run_model=function(times, Y0=NULL, parms=NULL, rtol=1e-6, atol=1e-6, maxsteps=5000,
                                                forcing=NULL, fcontrol=NULL, event_list=NULL, method="lsoda") {
                                 # Construct DLL name from mName.
                                 dll_name <- paste(mName, "_model", sep="")
                                 
                                 # If parameter values are not provided, use default values.
                                 if (is.null(parms)) {
                                   parms <- initParms()
                                 }
                                 
                                 # If initial values for state variables are not provided, use default
                                 # values.
                                 if (is.null(Y0)) {
                                   Y0 <- initStates(parms)
                                 }
                                 
                                 # Solve the ODE system using the "ode" function from the package "deSolve".
                                 out <- ode(Y0, times, func="derivs", parms=parms, rtol=rtol, atol=atol, maxsteps=maxsteps,
                                            dllname=dll_name, initforc="initforc", forcing=forcing,
                                            fcontrol=fcontrol, initfunc="initmod", nout=length(Outputs),
                                            outnames=Outputs, events=event_list, method=method)
                                 
                                 # Return the simulation output.
                                 return(out)
                             }
                               )
                             )
