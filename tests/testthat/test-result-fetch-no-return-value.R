test_that("A result set with N cols and 0 rows should return an 0xN data.frame.", {
  con <- DBI::dbConnect(
    RHyper::Hyper()
  )
  res <- DBI::dbGetQuery(conn = con, statement = SQL("SELECT 'test' AS x"))

  expected_result <- data.frame(x = character())

  expect_equal(object = res, expected = expected_result)
})
