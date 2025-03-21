#' MCSimMod class to run a model
#'
#' A class for managing MCSimMod models. This wrapper returns a `Model`
#' object to compile and run MCSim models.
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
#' @param mName path to model file (without .model extension)
#' @param mString string for creating model without .model file
#' @export
createModel <- function(mName = character(0), mString = character(0)) {
  return(Model(mName = mName, mString = mString))
}
