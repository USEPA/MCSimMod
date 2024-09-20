testthat::test_that("fixPath", {
  mNameNoSpace <- "path/to/my_model.model"
  mNameWithSpace <- "path/to/a spaced/my_model.model"

  mList <- .fixPath(mNameNoSpace)
  testthat::expect_equal(mList$mName, "my_model")
  testthat::expect_equal(mList$mPath, "path/to")
  testthat::expect_error(.fixPath(mNameWithSpace), "Error: User-defined directory has space which will throw error for .dll/.so compilation")
})
