testthat::test_that("Model$fromFile", {
  setwd('../data')
  testthat::expect_true(file.exists("exponential.model"))

  model <- Model(mName = "exponential")
  model$loadModel()
  model$updateParms(list(r = -0.5, A0 = 100))
  model$updateY0()

  times <- seq(from = 0, to = 10, by = 0.1)
  exp_out <- model$runModel(times)

  testthat::expect_true(all(dim(exp_out) == c(length(times), 4)))
  testthat::expect_true(all(colnames(exp_out) == c("time", "A", "Bout", "Cout")))
  testthat::expect_true(sum(exp_out[, 2]) > 0)

  model$cleanup()
})

testthat::test_that("Model$fromString", {
  modelString <- "
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
    "

  model <- Model(mString = modelString)
  model$loadModel()
  model$updateParms(list(r = -0.5, A0 = 100))
  model$updateY0()

  times <- seq(from = 0, to = 10, by = 0.1)
  output <- model$runModel(times)

  testthat::expect_true(all(dim(output) == c(length(times), 4)))
  testthat::expect_true(all(colnames(output) == c("time", "A", "Bout", "Cout")))
  testthat::expect_true(sum(output[, 2]) > 0)

  model$cleanup(deleteModel=T)
})
