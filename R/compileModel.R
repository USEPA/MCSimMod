#' Function to translate and compile MCSim model specification text
#'
#' This function translates MCSim model specification text to C and then
#' compiles the resulting C file to create a dynamic link library (DLL) file (on
#' Windows) or a shared object (SO) file (on Unix).
#'
#' @param model_file Name of an MCSim model specification file.
#' @param c_file Name of a C source code file to be created by compiling the MCSim model specification file.
#' @param dll_name Name of a DLL or SO file without the extension (".dll" or ".so").
#' @param dll_file Name of the same DLL or SO file with the appropriate extension (".dll" or ".so").
#' @param hash_file Name of a file containing a hash key for determining if `model_file` has changed since the previous translation and compilation.
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
