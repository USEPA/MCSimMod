testthat::test_that("Model$localModel", {
  current_dir <- getwd()
  setwd("./data")
  testthat::expect_true(file.exists("exponential.model"))

  model <- createModel("exponential")

  model$loadModel()
  model$updateParms(list(r = -0.5, A0 = 100))
  model$updateY0()

  times <- seq(from = 0, to = 10, by = 0.1)
  exp_out <- model$runModel(times)

  testthat::expect_true(all(dim(exp_out) == c(length(times), 4)))
  testthat::expect_true(all(colnames(exp_out) == c("time", "A", "Bout", "Cout")))
  testthat::expect_true(sum(exp_out[, 2]) > 0)

  model$cleanup()
  setwd(current_dir)
})

testthat::test_that("Model$relativeModel", {
  mName <- file.path(testthat::test_path(), "data", "exponential")
  testthat::expect_true(file.exists(paste0(mName, ".model")))

  model <- createModel(mName)

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

testthat::test_that("Model$absoluteModel", {
  # copy exponential.model to temp file -> /tmp/dir/new dir
  # Use absolute path of temp directory,
  # Test to make sure changing the file returns a changed path

  dir.create(file.path(tempdir(), "testDir"))
  mName <- tempfile(pattern = "mcsimmod_", tmpdir = file.path(tempdir(), "testDir"))
  mString <- readLines(file.path(testthat::test_path(), "data", "exponential.model"))
  writeLines(mString, paste0(mName, ".model"))

  testthat::expect_true(file.exists(paste0(mName, ".model")))
  model <- createModel(mName)

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

  model <- createModel(mString = modelString)

  model$loadModel()
  model$updateParms(list(r = -0.5, A0 = 100))
  model$updateY0()

  times <- seq(from = 0, to = 10, by = 0.1)
  output <- model$runModel(times)

  testthat::expect_true(all(dim(output) == c(length(times), 4)))
  testthat::expect_true(all(colnames(output) == c("time", "A", "Bout", "Cout")))
  testthat::expect_true(sum(output[, 2]) > 0)

  model$cleanup(deleteModel = T)
})
