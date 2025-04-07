#' Function to create an MCSimMod Model object
#'
#' This function creates a `Model` object using an MCSim model specification
#' file or an MCSim model specification string.
#'
#' @examples
#' \dontrun{
#' # Simple model
#' mod <- createModel("path/to/model")
#'
#' # Load/compile the model
#' mod$loadModel()
#'
#' # Update parameters (P1 and P2)
#' mod$updateParms(c(P1 = 3, P2 = 1))
#'
#' # Define times for ODE simulation
#' times <- seq(from = 0, to = 24, by = 0.1)
#'
#' # Run the simulation
#' out <- mod$runModel(times)
#' }
#'
#' @param mName Name of an MCSim model specification file, excluding the file name extension `.model`.
#' @param mString A character string containing MCSim model specification text.
#' @param writeTmp A boolean to write compiled files to tmp (writeTmp = TRUE) or to directory with `.model` file (writeTmp = FALSE).
#' @returns Model object.
#' @export
createModel <- function(mName = character(0), mString = character(0), writeTmp = TRUE) {
  return(Model(mName = mName, mString = mString, writeTmp = writeTmp))
}
