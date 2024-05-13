

cleanup <- function(model){
    name = model$mName
    dyn.unload(model$dll_file)
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

    exp_mod <- RMCSimModel('exponential')
    exp_mod <- load_model(exp_mod)
    exp_parms = exp_mod$initParms()
    exp_parms["r"] = -0.5
    exp_parms["A0"] = 100
    Y0_exp = exp_mod$initStates(exp_parms)
    times = seq(from=0, to=10, by=0.1)
    exp_out = run_model(exp_mod, times, Y0=Y0_exp, parms=exp_parms)

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
    model <- load_model(model)
    parms = model$initParms()
    parms["r"] = -0.5
    parms["A0"] = 100
    Y0 = model$initStates(parms)
    times = seq(from=0, to=10, by=0.1)
    output = run_model(model, times, Y0=Y0, parms=parms)

    testthat::expect_true(all(dim(output) == c(length(times), 4)))
    testthat::expect_true(all(colnames(output) == c("time", "A", "Bout", "Cout")))
    testthat::expect_true(sum(output[,2]) > 0)

    cleanup(model)
})
