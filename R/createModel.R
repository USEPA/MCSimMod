#' MCSimMod class to run a model
#'
#' A class for managing MCSimMod models
#'
#' @export
createModel <- function(mName = character(0), mString = character(0)) {
  return(Model(mName = mName, mString = mString))
}
