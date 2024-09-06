#' MCSimMod class to run a model
#'
#' A class for managing MCSimMod models
#'
#' @export
createModel <- function(mName=NULL, mString=NULL) {
  return(Model(mName=mName, mString=mString))

}