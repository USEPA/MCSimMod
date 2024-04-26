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

load_model <- function(model) {
  mName = model$mName
  # Construct names of required files and objects from mName.
  model$dll_name <- paste(mName, "_model", sep="")
  model$dll_file <- paste(model$dll_name, .Platform$dynlib.ext, sep="")
  model$inits_file <- paste(model$dll_name, "_inits.R", sep="")

  # Construct the compiled model names
  model$model_file = paste(mName, ".model", sep="")
  model$c_file = paste(mName, "_model.c", sep="")
  model$dll_name = paste(mName, "_model", sep="")
  model$dll_file = paste(model$dll_name, .Platform$dynlib.ext, sep="")

  if (!file.exists(model$dll_file)) {
    compile_model(model)
  }

  # Logic for compiling here if trying to load an uncompiled model

  # Load the compiled model (DLL).
  dyn.load(model$dll_file)

  # Run script that defines initialization functions.
  source(model$inits_file)
  model$initParms <- initParms
  model$initStates <- initStates
  model$Outputs <- Outputs
  model

  # Assign initialization functions and list of output variable names to the
  # "global" environment.
  #assign("initParms", initParms, envir=.GlobalEnv)
  #assign("initStates", initStates, envir=.GlobalEnv)
  #assign("Outputs", Outputs, envir=.GlobalEnv)
}
