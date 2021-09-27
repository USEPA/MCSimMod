#' This function runs a model that is defined using DLL and "_inits.R" files
#' based on translation of an MCSim model (".model") file. The model must first
#' be compiled (using the function "compile_model") and loaded into the R
#' workspace (using the function "load_model").
#'
#' Inputs:
#'   mName: String containing the name of the MCSim model. Exclude the file name
#'   suffix ".model". If the function is called from a working directory other
#'   than the one containing the ".model" file, the full path of the ".model"
#'   file should be provided.
#'
#'   times: Vector containing the times for which the model simulation results
#'   are desired. This vector should contain a minimum of two values (an initial
#'   time and an end time.
#'
#'   Y0: Vector containing the initial values of the state variables in the
#'   model. If this argument is NULL, the default initial values of the state
#'   variables (as returned by the function "initStates") will be used.
#'
#'   parms: Vector containing the values of the model parameters. If this
#'   argument is NULL, the default values of the parameters (as returned by the
#'   function "initParms") will be used.
#'
#'   r_tol: Relative error tolerance. See the documentation for the R package
#'   "deSolve".
#'
#'   a_tol: Absolute error tolerance. See the documentation for the R package
#'   "deSolve".
#'
#'   forcing: List containing values of input variables to be passed to the ODE
#'   solver. See the documentation for the R package "deSolve".
#'
#'   fcontrol: List containing arguments that describe how interpolations of the
#'   forcing functions should be performed. See the documentation for the R
#'   package "deSolve".
#'
#'   event_list: List containing events to be passed to the ODE solver. See
#'   the documentation for the R package deSolve.
#'
#'   method: Integration method to be used by "deSolve". Default solver is "lsoda"
#'
#' Returns:
#'   out: Dataframe containing values of all state variables (named in the
#'   argument "Y0") and all output variables (named in the global environment
#'   variable "Outputs") at all times (defined in the argument "times").
#'
#' @import deSolve
#' @export

run_model <- function(mName, times, Y0=NULL, parms=NULL, rtol=1e-6, atol=1e-6, maxsteps=5000,
                      forcing=NULL, fcontrol=NULL, event_list=NULL, method="lsoda") {


  # Construct DLL name from mName.
  dll_name = paste(mName, "_model", sep="")

  # If parameter values are not provided, use default values.
  if (is.null(parms)) {
    parms = initParms()
  }

  # If initial values for state variables are not provided, use default
  # values.
  if (is.null(Y0)) {
    Y0 = initStates(parms)
  }

  # Solve the ODE system using the "ode" function from the package "deSolve".
  out = ode(Y0, times, func="derivs", parms=parms, rtol=rtol, atol=atol, maxsteps=maxsteps,
            dllname=dll_name, initforc="initforc", forcing=forcing,
            fcontrol=fcontrol, initfunc="initmod", nout=length(Outputs),
            outnames=Outputs, events=event_list, method=method)

  # Return the simulation output.
  return(out)
}
