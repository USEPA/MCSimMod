#' MCSimMod class to run a model
#'
#' A class for managing MCSimMod models
#'
#' @param mName path to model file (without .model extension)
#' @param mString string for creating model without .model file
#' @export
createModel <- function(mName = character(0), mString = character(0)) {
  return(Model(mName = mName, mString = mString))
}
