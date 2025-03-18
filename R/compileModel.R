#' Compiles a model file into an executable dynamic linked library
#' 
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
#' @param model_file model file name that needs to be compiled
#' @param c_file output c_file that is compiled by `c_mod`
#' @param dll_name dynamic library that has the "derivs" function from a previously compiled model
#' @param dll_file Possible previously compiled dll
#' @param hash_file Location of hash key for determining if .model file has changed
#'
#' @import tools
#' @useDynLib MCSimMod, .registration=TRUE
#' @export
compileModel <- function(model_file, c_file, dll_name, dll_file, hash_file = NULL) {
  # Unload DLL if it has been loaded.
  if (is.loaded("derivs", PACKAGE = dll_name)) {
    dyn.unload(dll_file)
  }

  # Create a C model file (ending with ".c") and an R parameter
  # initialization file (ending with "_inits.R") from the GNU MCSim model
  # definition file (ending with ".model"). Using the "-R" option generates
  # code compatible with functions in the R deSolve package.

  # if (.C("c_mod", model_file, c_file)[[1]] < 0) {
  if (is.null(.C("c_mod", model_file, c_file)[[2]])) {
    stop("c_mod failed")
  }

  # Compile the C model to obtain "mName_model.o" and "mName_model.dll".
  r_path <- file.path(R.home("bin"), "R")
  system(paste0(r_path, " CMD SHLIB ", c_file))

  if (!is.null(hash_file)) {
    file_hash <- as.character(md5sum(model_file))
    write(file_hash, file = hash_file)
    cat("Hash calculated and saved to", hash_file, "\n")
  }
}
