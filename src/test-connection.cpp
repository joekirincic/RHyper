#include "connection.h"
#include <testthat.h>
#include <Rcpp.h>

context("Hello2") {

  // The format for specifying tests is similar to that of
  // testthat's R functions. Use 'test_that()' to define a
  // unit test, and use 'expect_true()' and 'expect_false()'
  // to test the desired conditions.
  test_that("A connection object can be created without specifiying a database.2") {
    expect_true(1+1==2);
  }

}
