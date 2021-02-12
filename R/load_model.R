#' This function loads a model that is defined using DLL and "_inits.R" files
#' based on translation of an MCSim model (".model") file. The R functions
#' "initParms" and "initStates" and the R vector "Outputs" are defined and
#' assigned to the "global" environment.
#'
#' Inputs:
#'   mName: String containing the name of the MCSim model. Exclude the file name
#'   suffix ".model". If the function is called from a working directory other
#'   than the one containing the ".model" file, the full path of the ".model"
#'   file should be provided.
#'
#' @export

load_model <- function(mName) {
  # Construct names of required files and objects from mName.
  dll_name = paste(mName, "_model", sep="")
  dll_file = paste(dll_name, .Platform$dynlib.ext, sep="")
  inits_file = paste(dll_name, "_inits.R", sep="")

  # Load the compiled model (DLL).
  dyn.load(dll_file)

  # Run script that defines initialization functions.
  source(inits_file)

  # Assign initialization functions and list of output variable names to the
  # "global" environment.
  assign("initParms", initParms, envir=.GlobalEnv)
  assign("initStates", initStates, envir=.GlobalEnv)
  assign("Outputs", Outputs, envir=.GlobalEnv)
}
