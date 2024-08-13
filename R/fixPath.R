#-----------------
# fixPath
#----------------
# Private function to create platform-dependent mPath and mName 
# based on absolute path generate by normalizePath

.fixPath <- function(file) {
  new.mName <- strsplit(basename(file), "[.]")[[1]][1]
  new.mPath <- dirname(file)
  if (.Platform$OS.type == 'windows') {
    new.mPath <- gsub('\\\\', '/', utils::shortPathName(new.mPath))
  }
  return(list("mPath"=new.mPath, "mName"=new.mName))
}
