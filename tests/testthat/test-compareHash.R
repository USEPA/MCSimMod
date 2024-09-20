testthat::test_that("test_compareHash", {
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
  testthat::expect_true(file.exists(model$paths$hash_file)) # Check if hash was created
  # File hasn't changed so hash should be the same
  testthat::expect_false(.compareHash(model$paths$model_file, model$paths$hash_file))
  # Add a new line to the temp model to change it
  line = '# Changed model file'
  write(line, file = model$paths$model_file, append = T, sep = '\n')
  testthat::expect_true(.compareHash(model$paths$model_file, model$paths$hash_file))

  model$cleanup()
})