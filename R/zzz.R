.onLoad <- function(libname, pkgname) {
  if(Sys.which("gcc") == ""){
    stop("Please set the PATH of compiler")
  }
  #system(paste("gcc -o ./inst/bin/mod.exe ./inst/mod/*.c ", sep = ""))

  if(file.exists(paste(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod", sep="/"), "exe", sep='.'))){
    message("The mod.exe had been created.")
  } else {
    system(paste("gcc -o", shQuote(paste(file.path(system.file(package = "RMCSim"), "bin"), "mod.exe", sep="/")), shQuote(paste(file.path(system.file(package = "RMCSim"), "mod"), "*.c", sep="/")), sep=" "))
  }

}
