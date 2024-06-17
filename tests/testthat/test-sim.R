

cleanup <- function(model){
    name = model$mName
    dll_name <- paste(name, "_model", sep="")
    dll_file <- paste(dll_name, .Platform$dynlib.ext, sep="")
    dyn.unload(dll_file)
    rm(model)
    file.remove(paste0(name,'_model.o'))
    file.remove(paste0(name,'_model.c'))
    file.remove(paste0(name,'_model_inits.R'))
    file.remove(paste0(name,'_model.dll'))
    file.remove(paste0(name,'_model.so'))

    prefix = "tmp_mcsim"
    if (substr(name,1,nchar(prefix)) == prefix){
        file.remove(paste0(name,'.model'))
    }
}

testthat::test_that("Model", {

    setwd('../../inst/extdata')
    testthat::expect_true(file.exists('exponential.model'))

    exp_mod <- Model$new(mName='exponential')
    exp_mod$loadModel() # Default parms are loaded to exp_mod object
    
    # Update parms for user-defined parameters
    exp_mod$updateParms(list(r=-0.5, A0=100))
    
    # Update initial states
    exp_mod$updateY0()
    
    times = seq(from=0, to=10, by=0.1)
    exp_out = exp_mod$runModel(times)

    testthat::expect_true(all(dim(exp_out) == c(length(times), 4)))
    testthat::expect_true(all(colnames(exp_out) == c("time", "A", "Bout", "Cout")))
    testthat::expect_true(sum(exp_out[,2]) > 0)

    cleanup(exp_mod)
})

testthat::test_that("fromString", {
    model = fromString('
    States = {A};
    Outputs = {Bout, Cout};
    Inputs = {Bin, Cin};
    A0 = 1e-6;
    r = 1.4;

    Initialize {
        A = A0;
    }

    Dynamics {
        Bout = Bin;
        Cout = Cin;
        dt(A) = r * A;
    }

    End.
    ')
    model$loadModel()
    
    model$updateParms(list(r=-0.5, A0=100))
    
    model$updateY0()
    
    times = seq(from=0, to=10, by=0.1)
    output = model$runModel(times)

    testthat::expect_true(all(dim(output) == c(length(times), 4)))
    testthat::expect_true(all(colnames(output) == c("time", "A", "Bout", "Cout")))
    testthat::expect_true(sum(output[,2]) > 0)

    cleanup(model)
})
