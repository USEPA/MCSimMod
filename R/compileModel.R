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
#' @returns No return value. Creates files and saves them in locations specified by function arguments.
#' @import tools
#' @useDynLib MCSimMod, .registration=TRUE
#' @export
compileModel <- function(model_file, c_file, dll_name, dll_file, hash_file = NULL) {
  # Unload DLL if it has been loaded.
  if (is.loaded("derivs", PACKAGE = dll_name)) {
    dyn.unload(dll_file)
  }

  # Create a text connection to store output messages generated during the
  # translation from MCSim model specification text to C.
  text_conn <- textConnection("mod_output", open = "w")

  # Create a C model file (ending with ".c") and an R parameter
  # initialization file (ending with "_inits.R") from the GNU MCSim model
  # specification file (ending with ".model"). Using the "-R" option generates
  # code compatible with functions in the R deSolve package. Write translator
  # output to the text connection.
  sink(text_conn)
  .C("c_mod", model_file, c_file)
  sink()
  close(text_conn)
  mod_output <- paste(mod_output, collapse = "\n")


  # Check to see if there was an error during translation. If so, print the
  # translator output and stop execution.
  if (grepl("Error", mod_output)) {
    temp_directory <- tempdir()
    out_file <- file.path(temp_directory, "mod_output.txt")
    write(mod_output, file = out_file)
    stop(
      "There was a problem with translating the MCSim model specification ",
      "text to C. Full details are available in the file ",
      normalizePath(out_file), "."
    )
  }

  # Compile the C model to obtain an object file (ending with ".o") and a
  # machine code file (ending with ".dll" or ".so").
  r_path <- file.path(R.home("bin"), "R")
  system(paste0(r_path, " CMD SHLIB ", c_file),
    ignore.stdout = TRUE,
    ignore.stderr = TRUE
  )
  message("Compiled model file created: ", normalizePath(dll_file))

  if (!is.null(hash_file)) {
    file_hash <- as.character(md5sum(model_file))
    write(file_hash, file = hash_file)
    message("Hash file created: ", normalizePath(hash_file))
  }
}
