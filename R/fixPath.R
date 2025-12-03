#-----------------
# fixPath
#----------------
# Private function to create platform-dependent mPath and mName
# based on absolute path generate by normalizePath

.fixPath <- function(file) {
  new.mName <- strsplit(basename(file), "[.]")[[1]][1]
  new.mPath <- dirname(file)
  if (.Platform$OS.type == "windows") {
    # Normalize path and convert backslashes to forward slashes
    new.mPath <- normalizePath(new.mPath, winslash = "/", mustWork = FALSE)
    # Only use shortPathName if there are spaces and it's needed
    if (grepl(" ", new.mPath)) {
      tryCatch({
        short_path <- utils::shortPathName(new.mPath)
        if (short_path != "" && short_path != new.mPath) {
          new.mPath <- gsub("\\\\", "/", short_path)
        }
      }, error = function(e) {
        # If shortPathName fails, keep the original path
        # The compilation might still work with quoted paths
      })
    }
  }

  has_space <- grepl(" ", new.mPath)
  if (has_space == TRUE) {
    warning("Directory path contains spaces which may cause compilation issues: ", new.mPath)
  }

  return(list("mPath" = new.mPath, "mName" = new.mName))
}
