testthat::test_that("compileModel::error", {
  modelString <- "bad model"
  model <- createModel(mString = modelString)
  testthat::expect_error(
    model$loadModel(),
    "An error was identified when translating the MCSim model specification"
  )
})

testthat::test_that("compileModel::warning", {
  # defining the input Bin as a dynamic will trigger a warning
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
        Bin = 2;
        Bout = Bin;
        Cout = Cin;
        dt(A) = r * A;
    }

    End.
    "

  model <- createModel(mString = modelString)
  testthat::expect_warning(
    model$loadModel(),
    "A warning was identified when translating the MCSim model specification"
  )
})
