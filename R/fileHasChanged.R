#-----------------
# compareHash
#----------------
# Private function to determine if the .model file has changed

.fileHasChanged <- function(model_file, hash_file) {
  
  # Calculate hash for current model file
  current_hash <- as.character(md5sum(model_file))
  
  # Read saved hash
  saved_hash <- readLines(hash_file, n = 1)

  # Compare the hashed
  has_changed <- current_hash != saved_hash
  return(has_changed)
}
