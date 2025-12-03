# Comprehensive tests for writeTemp bug fix covering two key scenarios:
# Scenario 1: Local .model file with writeTemp=TRUE - change detection and recompilation
# Scenario 2: mString models - change detection when temp file is modified

testthat::test_that("writeTemp=TRUE with local file: change detection and recompilation", {
  # Create a model file outside of tempdir to properly test writeTemp behavior
  test_dir <- file.path(dirname(tempdir()), "test_mcsim_writeTemp")
  dir.create(test_dir, showWarnings = FALSE, recursive = TRUE)
  original_model_path <- file.path(test_dir, "test_model.model")

  # Create a simple test model
  model_content <- c(
    "States = {Q_central};",
    "Inputs = {Q_input};",
    "Outputs = {Q_out};",
    "",
    "Initialize {",
    "  Q_central = 0;",
    "}",
    "",
    "Dynamics {",
    "  Q_out = Q_central;",
    "  dt(Q_central) = Q_input - 0.1 * Q_central;",
    "}",
    "",
    "End."
  )
  writeLines(model_content, original_model_path)

  # Test 1: Create model with writeTemp=TRUE using local file (Scenario 1)
  model <- createModel(mName = file.path(test_dir, "test_model"), writeTemp = TRUE)

  # Verify paths are set correctly
  testthat::expect_true(file.exists(model$paths$model_file))
  testthat::expect_equal(model$paths$model_file, original_model_path)
  testthat::expect_true(grepl(tempdir(), model$paths$source_file))

  # Hash file should be alongside model file, not in temp directory
  expected_hash_file <- file.path(test_dir, "test_model_model.md5")
  testthat::expect_equal(model$paths$hash_file, expected_hash_file)

  # Test 2: Load model - should create hash file
  testthat::expect_false(file.exists(model$paths$hash_file))
  model$loadModel()
  testthat::expect_true(file.exists(model$paths$hash_file))

  # Test 3: Modify original file
  modified_content <- c(
    "States = {y};",
    "y0 = 0;",
    "m = 0.2;", # Changed from 0.1 to 0.2
    "Initialize {",
    "  y = y0;",
    "}",
    "Dynamics {",
    "  dt(y) = m;",
    "}",
    "End."
  )
  writeLines(modified_content, original_model_path)

  # Test 4: Load model again - should detect change and recompile (Scenario 1 verification)
  # First, get the current hash
  old_hash <- readLines(model$paths$hash_file, n = 1)

  # Load model again - this should:
  # 1. Detect model_file has changed (via hash comparison)
  # 2. Update source_file (working copy) from model_file
  # 3. Recompile the updated source_file
  # 4. Create new hash from the compiled source_file
  model$loadModel()

  # Verify hash was updated (indicates recompilation occurred)
  new_hash <- readLines(model$paths$hash_file, n = 1)
  testthat::expect_false(old_hash == new_hash)

  # Verify working copy was updated with new content from model_file
  working_content <- readLines(model$paths$source_file)
  testthat::expect_true("m = 0.2;" %in% working_content)

  # Cleanup
  model$cleanup(deleteModel = TRUE)
  if (file.exists(model$paths$hash_file)) {
    file.remove(model$paths$hash_file)
  }
  if (file.exists(original_model_path)) {
    file.remove(original_model_path)
  }
  # Clean up test directory
  unlink(test_dir, recursive = TRUE)
})

testthat::test_that("writeTemp=FALSE behavior unchanged", {
  # Create a temporary model file to test with
  temp_dir <- tempdir()
  model_path <- file.path(temp_dir, "test_model_false.model")

  # Create a simple test model using intro.Rmd nomenclature
  model_content <- c(
    "States = {y};",
    "y0 = 0;",
    "m = 0.1;",
    "Initialize {",
    "  y = y0;",
    "}",
    "Dynamics {",
    "  dt(y) = m;",
    "}",
    "End."
  )
  writeLines(model_content, model_path)

  # Test 1: Create model with writeTemp=FALSE
  model <- createModel(mName = file.path(temp_dir, "test_model_false"), writeTemp = FALSE)

  # Verify source_file and model_file are the same
  testthat::expect_equal(model$paths$source_file, model$paths$model_file)
  testthat::expect_equal(model$paths$model_file, model_path)

  # Hash file should be in same directory as model file
  expected_hash_file <- file.path(temp_dir, "test_model_false_model.md5")
  testthat::expect_equal(model$paths$hash_file, expected_hash_file)

  # Test 2: Load model - should work as before
  model$loadModel()
  testthat::expect_true(file.exists(model$paths$hash_file))

  # Cleanup
  model$cleanup(deleteModel = TRUE)
  if (file.exists(model$paths$hash_file)) {
    file.remove(model$paths$hash_file)
  }
})

testthat::test_that("mString behavior and change detection", {
  # Test model specification as string using intro.Rmd nomenclature
  model_string <- paste(c(
    "States = {y};",
    "y0 = 0;",
    "m = 0.1;",
    "Initialize {",
    "  y = y0;",
    "}",
    "Dynamics {",
    "  dt(y) = m;",
    "}",
    "End."
  ), collapse = "\n")

  # Test 1: Create model with mString (writeTemp must be TRUE)
  model <- createModel(mString = model_string, writeTemp = TRUE)

  # For mString, source_file and model_file should be the same (both temp files)
  testthat::expect_equal(model$paths$source_file, model$paths$model_file)
  testthat::expect_true(grepl(tempdir(), model$paths$model_file))

  # Hash file should be with the temp file
  testthat::expect_true(grepl(tempdir(), model$paths$hash_file))

  # Test 2: Load model and capture initial hash
  model$loadModel()
  testthat::expect_true(file.exists(model$paths$hash_file))
  original_hash <- readLines(model$paths$hash_file, n = 1)

  # Test 3: Simulate change to mString-created temp file (Scenario 2 verification)
  # Since mString creates a temp file, we test change detection by modifying that temp file
  # This verifies hash-based change detection works for mString models
  temp_file_content <- readLines(model$paths$model_file)
  modified_content <- gsub("m = 0.1;", "m = 0.2;", temp_file_content)
  writeLines(modified_content, model$paths$model_file)

  # Reload model - should detect change and recompile
  model$loadModel()
  new_hash <- readLines(model$paths$hash_file, n = 1)
  testthat::expect_false(original_hash == new_hash)

  # Test 4: Verify mString + writeTemp=FALSE still throws error
  testthat::expect_error(
    createModel(mString = model_string, writeTemp = FALSE),
    "The value of writeTemp must be TRUE when creating a Model object using a model specification string"
  )

  # Cleanup
  model$cleanup(deleteModel = TRUE)
})
