#' This function translates a model that has been defined in an MCSim model
#' (".model") file into the C language (i.e., a ".c" file). It then compiles the
#' model to create an object code (".o") file and a dynamic linked library
#' (".dll") file, as well as an R script ("_inits.R") containing several R
#' functions that can be used for initializing model states and parameters.
#'
#' Inputs:
#'   mName: String containing the name of the MCSim model. Exclude the file name
#'   suffix ".model". If the function is called from a working directory other
#'   than the one containing the ".model" file, the full path of the ".model"
#'   file should be provided.
#'
#' @export
compile_model <- function(model) {
  mName = model$mName
  # Construct names of required files and objects from mName.
  model_file <- model$model_file
  c_file <- model$c_file
  dll_name <- model$dll_name
  dll_file <- model$dll_file


  # Unload DLL if it has been loaded.
  if (is.loaded("derivs", PACKAGE=dll_name)) {
    dyn.unload(dll_file)
  }

  # Create a C model file (ending with ".c") and an R parameter
  # initialization file (ending with "_inits.R") from the GNU MCSim model
  # definition file (ending with ".model"). Using the "-R" option generates
  # code compatible with functions in the R deSolve package.
  #system(paste("mod -R ", model_file, " ", c_file, sep = ""))

  if (.Platform$OS.type == 'windows') {
    system(paste(shQuote(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/")), paste(" -R ", model_file, " ", c_file, sep = ""), sep=''))
  } else if (.Platform$OS.type == 'unix'){
    system(paste(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/"), paste(" -R ", model_file, " ", c_file, sep = ""), sep=''))
  } else {
    message("RMCSim only available for windows or unix OS")
  }

  # Not needed for compiled executable
  #.Call('mod', model_file, c_file, PACKAGE='RMCSim')
  #.C("mod", model_file, c_file, 'RMCSim.so')
  
  # Compile the C model to obtain "mName_model.o" and "mName_model.dll".
  system(paste("R CMD SHLIB ", c_file, sep = ""))
}
