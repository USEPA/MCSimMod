#-----------------
# compareHash
#----------------
# Private function to determine if the source .model file has changed

.fileHasChanged <- function(source_file, hash_file) {
  # Calculate hash for current model file
  current_hash <- tools::md5sum(source_file)

  # Read saved hash
  saved_hash <- readLines(hash_file, n = 1)

  # Compare the hashed
  has_changed <- current_hash != saved_hash
  return(has_changed)
}
