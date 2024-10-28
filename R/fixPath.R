#-----------------
# fixPath
#----------------
# Private function to create platform-dependent mPath and mName
# based on absolute path generate by normalizePath

.fixPath <- function(file) {
  new.mName <- strsplit(basename(file), "[.]")[[1]][1]
  new.mPath <- dirname(file)
  if (.Platform$OS.type == "windows") {
    new.mPath <- gsub("\\\\", "/", utils::shortPathName(new.mPath))
  }

  has_space <- grepl(" ", new.mPath)
  if (has_space == T) {




    stop(
      "Error: User-defined directory has space which will throw error for .dll/.so compilation"


    )
  }

  return(list("mPath" = new.mPath, "mName" = new.mName))
}
