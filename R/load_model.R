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
load_model <- function() {
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
  
}
