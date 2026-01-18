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
  testthat::expect_false(.fileHasChanged(model$paths$model_file, model$paths$hash_file))
  # Add a new line to the temp model to change it
  line <- "# Changed model file"
  write(line, file = model$paths$model_file, append = T, sep = "\n")
  testthat::expect_true(.fileHasChanged(model$paths$model_file, model$paths$hash_file))

  model$cleanup()
})

testthat::test_that("test_tmp_compileModel", {
  # Test writeTemp=TRUE functionality with proper source/compilation file separation
  # Create a proper source file in a permanent location (not temp)
  source_dir <- file.path(tempdir(), "sourceDir")
  dir.create(source_dir, showWarnings = FALSE)
  mName <- file.path(source_dir, "test_model")
  mString <- readLines(file.path(testthat::test_path(), "data", "exponential.model"))
  writeLines(mString, paste0(mName, ".model"))
  
  # Verify source file exists
  testthat::expect_true(file.exists(paste0(mName, ".model")))
  
  # Create model with writeTemp=TRUE - this should use the source file 
  # and create compilation files in temp directory
  model <- createModel(mName, writeTemp=TRUE)
  
  # Verify paths are properly separated
  testthat::expect_true(model$paths$source_file != model$paths$model_file)
  testthat::expect_true(file.exists(model$paths$source_file))
  testthat::expect_true(file.exists(model$paths$model_file))
  
  # First load - should compile
  model$loadModel()
  testthat::expect_true(file.exists(model$paths$hash_file)) # Check if hash was created
  testthat::expect_true(model$recompiled) # Should be TRUE on first compile
  
  # Second load - should NOT recompile (no changes)
  model$loadModel()
  testthat::expect_false(.fileHasChanged(model$paths$source_file, model$paths$hash_file))
  testthat::expect_false(model$recompiled) # Should be FALSE, no recompile needed
  
  # Edit source file (what users actually edit)
  write("# File is edited", file = model$paths$source_file, append = TRUE, sep = '\n')
  testthat::expect_true(.fileHasChanged(model$paths$source_file, model$paths$hash_file))
  
  # Third load after edit - should recompile
  model$loadModel()
  testthat::expect_true(model$recompiled) # Should be TRUE, source changed
  
  # Fourth load - should NOT recompile (our bug fix test!)
  model$loadModel()
  testthat::expect_false(.fileHasChanged(model$paths$source_file, model$paths$hash_file))
  testthat::expect_false(model$recompiled) # Should be FALSE, hash should now match
  
  model$cleanup()
})
