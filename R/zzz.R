.onLoad <- function(libname, pkgname) {
  if(Sys.which("gcc") == ""){
    stop("Please set the PATH of compiler")
  }
  #system(paste("gcc -o ./inst/bin/mod.exe ./inst/mod/*.c ", sep = ""))

  if(file.exists(paste(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/"), "exe", sep='.'))){
    message("The mod.exe had been created on Windows.")
  } else if (file.exists(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/"))) {
    message("The mod had been created on Linux.")
  } else {
    if (.Platform$OS.type == 'windows') {
      c.files <- c("getopt.c", "lex.c", "lexerr.c", "lexfn.c", "mod.c", "modd.c", "modi.c", "modiSBML.c", "modiSBML2.c", "modo.c", "strutil.c")
      path.to.files = paste(file.path(system.file(package = "RMCSim"), "mod"), '/', sep="")
      #system(paste("gcc -o", shQuote(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod.exe", sep="/")), shQuote(paste(file.path(system.file(package = "RMCSim"), "mod"), c.files, sep="/")), sep=" "))
      system(paste("gcc", paste0('"', path.to.files, c.files, '"', collapse = " "), '-o', shQuote(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod.exe", sep="/")), sep=" "))
    }
    else if (.Platform$OS.type == 'unix') {
      system(paste("gcc -o", paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/"), paste(file.path(system.file(package = "RMCSim"), "mod"), "*.c", sep="/"), sep=" "))
    }
    else {
      message("RMCSim only available for windows or unix OS")
    }
  }

}
