

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

    prefix = "tmp_mcsim"
    if (substr(name,1,nchar(prefix)) == prefix){
        file.remove(paste0(name,'.model'))
    }
}

testthat::test_that("RMCSimModel", {

    setwd('../data')
    testthat::expect_true(file.exists('exponential.model'))

    exp_mod <- RMCSimModel$new(mName='exponential')
    exp_mod$load_model() # Default parms are loaded to exp_mod object
    
    # Update parms for user-defined parameters
    exp_mod$update_parms(list(r=-0.5, A0=100))
    
    # Update initial states
    exp_mod$update_Y0()
    
    times = seq(from=0, to=10, by=0.1)
    exp_out = exp_mod$run_model(times)

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
    model$load_model()
    
    model$update_parms(list(r=-0.5, A0=100))
    
    model$update_Y0()
    
    times = seq(from=0, to=10, by=0.1)
    output = model$run_model(times)

    testthat::expect_true(all(dim(output) == c(length(times), 4)))
    testthat::expect_true(all(colnames(output) == c("time", "A", "Bout", "Cout")))
    testthat::expect_true(sum(output[,2]) > 0)

    cleanup(model)
})
