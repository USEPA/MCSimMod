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

  # If there was an error during translation, bubble up to an R exception
  # and show user location of stdout file to investigate.
  if (grepl("\\*\\*\\* Error:", mod_output)) {
    temp_directory <- tempdir()
    out_file <- file.path(temp_directory, "mod_output.txt")
    write(mod_output, file = out_file)
    stop(
      "There was an error translating the MCSim model specification ",
      "to C. Full details are available in the file ",
      normalizePath(out_file), "."
    )
  }

  # If there was a warning during translation, bubble up to an R warning
  # and show user location of stdout file to investigate.
  if (grepl("\\*\\*\\* Warning:", mod_output)) {
    temp_directory <- tempdir()
    out_file <- file.path(temp_directory, "mod_output.txt")
    write(mod_output, file = out_file)
    warning(
      "There was a warning translating the MCSim model specification ",
      "to C. Full details are available in the file ",
      normalizePath(out_file), "."
    )
  }

  # Compile the C model to obtain an object file (ending with ".o") and a
  # machine code file (ending with ".dll" or ".so"). Write compiler output
  # to a character string.
  r_path <- file.path(R.home("bin"), "R")
  compiler_output <- system(paste(shQuote(r_path), "CMD SHLIB", shQuote(c_file)), intern = TRUE)

  # Save the compiler output to a file and print a message about its location.
  temp_directory <- tempdir()
  out_file <- file.path(temp_directory, "compiler_output.txt")
  write(compiler_output, file = out_file)
  message(
    "C compilation complete. Full details are available in the file ",
    normalizePath(out_file), "."
  )

  # If hash file name was provided, create a hash (md5 sum) for the model file
  # and print a message about its location.
  if (!is.null(hash_file)) {
    file_hash <- as.character(md5sum(model_file))
    write(file_hash, file = hash_file)
    message(
      "Hash created and saved in the file ", normalizePath(hash_file),
      "."
    )
  }
}
