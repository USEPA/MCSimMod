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
#' @param source_file Name of the original source file to use for hash calculation. Defaults to \code{model_file} for backward compatibility. When \code{writeTemp=TRUE} in \code{createModel()}, this should be set to the original source file path to ensure hash tracking works correctly when the source file is separate from the compiled model file.
#' @param hash_file Name of a file containing a hash key for determining if `source_file` has changed since the previous translation and compilation.
#' @param verbose_output Boolean specifying whether to write translator messages to standard output. If value is TRUE, messages will be written to standard output; if value is FALSE, messages will be written to files in a temporary directory.
#' @returns No return value. Creates files and saves them in locations specified by function arguments.
#' @import tools
#' @useDynLib MCSimMod, .registration=TRUE
#' @export
compileModel <- function(model_file, c_file, dll_name, dll_file, source_file = model_file, hash_file = NULL, verbose_output = FALSE) {
  # Unload DLL if it has been loaded.
  mList <- .fixPath(model_file)
  model_name <- mList$mName
  model_path <- mList$mPath
  derivs_name <- paste0("derivs_", model_name)
  if (is.loaded(derivs_name, PACKAGE = dll_name)) {
    dyn.unload(dll_file)
  }

  # Create a text connection to store output messages generated during the
  # translation from MCSim model specification text to C.
  text_conn <- textConnection("mod_output", open = "w")

  # Create a C model file (ending with ".c") and an R parameter
  # initialization file (ending with "_inits.R") from the GNU MCSim model
  # specification file (ending with ".model"). Write translator output to the
  # text connection.
  sink(text_conn)
  .C("c_mod", model_file, c_file)
  sink()
  close(text_conn)
  mod_output <- paste(mod_output, collapse = "\n")

  # Save the translator output to a file.
  if (!verbose_output) {
    temp_directory <- tempdir()
    out_file <- file.path(temp_directory, "mod_output.txt")
    write(mod_output, file = out_file)
  }

  # Check to see if there was an error during translation. If so, print a
  # message about where to find full details and stop execution.
  if (grepl("*** Error:", mod_output, fixed = TRUE)) {
    if (verbose_output) {
      stop(
        "An error was identified when translating the MCSim model ",
        "specification text to C. Full details follow.\n", mod_output
      )
    } else {
      stop(
        "An error was identified when translating the MCSim model ",
        "specification text to C. Full details are available in the file ",
        normalizePath(out_file), "."
      )
    }
  }

  # Check to see if there was a warning during translation. If so, print a
  # message about the location of the translation log file and raise a warning.
  else if (grepl("*** Warning:", mod_output, fixed = TRUE)) {
    if (verbose_output) {
      warning(
        "A warning was generated when translating the MCSim model ",
        "specification text to C. Full details follow.\n", mod_output
      )
    } else {
      warning(
        "A warning was generated when translating the MCSim model ",
        "specification text to C. Full details are available in the file ",
        normalizePath(out_file), ".\n"
      )
    }
  }

  # If there was no error or warning during translation, just print a message
  # about the location of the translation log file.
  else {
    if (verbose_output) {
      message(
        "Translation of model specification text complete. Full details ",
        "follow.\n", mod_output
      )
    } else {
      message(
        "Translation of model specification text complete. Full details are ",
        "available in the file ",
        normalizePath(out_file), ".\n"
      )
    }
  }

  # Code to update C source file using model-specific names for objects.

  # Read the original C source file.
  lines <- readLines(c_file)

  # Find and replace C object names with model-specific names.
  item_to_replace <- c(
    "parms",
    "forc",
    "Nout",
    "nr",
    "ytau",
    "yini",
    "lagvalue",
    "CalcDelay",
    "initmod",
    "initforc",
    "initState",
    "getParms",
    "derivs",
    "jac",
    "event",
    "root"
  )
  for (idx in seq_along(item_to_replace)) {
    lines <- gsub(
      paste0("\\b", item_to_replace[idx], "\\b"),
      paste0(item_to_replace[idx], "_", model_name),
      lines
    )
  }

  # Overwrite the C source file with the updated text.
  writeLines(lines, c_file)


  # Code to update inits R source file using model-specific names for objects.

  # Get inits R source file name.
  inits_file <- file.path(
    model_path,
    paste0(model_name, "_model_inits.R")
  )

  # Read the original inits R source file.
  lines <- readLines(inits_file)

  # Find and replace R and C object names with model-specific names.
  item_to_replace <- c(
    "initParms",
    "getParms",
    "Outputs",
    "initStates",
    "initState"
  )
  for (idx in seq_along(item_to_replace)) {
    lines <- gsub(
      paste0("\\b", item_to_replace[idx], "\\b"),
      paste0(item_to_replace[idx], "_", model_name),
      lines
    )
  }

  # Overwrite the R inits source file with the updated text.
  writeLines(lines, inits_file)


  # Compile the C model to obtain an object file (ending with ".o") and a
  # machine code file (ending with ".dll" or ".so"). Write compiler output
  # to a character string.
  r_path <- file.path(R.home("bin"), "R")
  compiler_output <- system(paste(
    shQuote(r_path), "CMD SHLIB",
    shQuote(c_file)
  ), intern = TRUE)

  # Save the compiler output to a file and print a message about its location.
  temp_directory <- tempdir()
  out_file <- file.path(temp_directory, "compiler_output.txt")
  write(compiler_output, file = out_file)
  message(
    "C compilation complete. Full details are available in the file ",
    normalizePath(out_file), ".\n"
  )

  # If hash file name was provided, create a hash (md5 sum) for the source file
  # and print a message about its location.
  if (!is.null(hash_file)) {
    file_hash <- as.character(tools::md5sum(source_file))
    write(file_hash, file = hash_file)
    message(
      "Hash created and saved in the file ", normalizePath(hash_file),
      ".\n"
    )
  }
}
