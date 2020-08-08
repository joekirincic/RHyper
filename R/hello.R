# Hello, world!
#
# This is an example function named 'hello'
# which prints 'Hello, world!'.
#
# You can learn more about package authoring with RStudio at:
#
#   http://r-pkgs.had.co.nz/
#
# Some useful keyboard shortcuts for package authoring:
#
#   Install Package:           'Cmd + Shift + B'
#   Check Package:             'Cmd + Shift + E'
#   Test Package:              'Cmd + Shift + T'

hello <- function() {
  print("Hello, world!")
}

#' @export
buggy_fun <- function(){
  library(tidyverse)
  library(DBI)

  source_db <- here::here("test.hyper")

  con <- dbConnect(
    RHyper::Hyper()
  )

  dbExecute(conn = con, statement = glue::glue_sql('ATTACH DATABASE {`source_db`} AS "source"', .con = ANSI()))

  res <- dbSendQuery(conn = con, statement = SQL('SELECT * FROM mtcars'))

  df <- dbFetch(res = res, n = 99999)

}
