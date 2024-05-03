#' RMCSim class to run a model
#' 
#' A class for managing RMCSim models
#' 
#' @export
RMCSimModel <- function(model_name) {
    obj <- list()
    class(obj) <- "RMCSimModel"
    obj$mName <- model_name
    obj$dll_name <- NULL
    obj$dll_file <- NULL
    obj$inits_file <- NULL

    obj$initParms <- NULL
    obj$initStates <- NULL
    obj$Outputs <- NULL
    return(obj)
}

#' Return an RMCSim instance from a string-based model
#'
#' Convenience method to build a string-based method.
#'
#' @export
fromString <- function(string){
    file <- tempfile(pattern="tmp_mcsim", tmpdir='.')
    writeLines(string, paste0(file, ".model"))
    model = RMCSimModel(basename(file))
    return(model)
}
