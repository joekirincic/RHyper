
#' Refer to a table in a specific database.
#'
#' @param database
#' @param schema
#' @param table
#'
#' @export
#'
in_database <- function(database, schema = "public", table) {
  structure(
    list(database = database, schema = schema, table = table),
    class = "dbplyr_database"
  )
}

#' @export
print.dbplyr_database <- function(x, ...) {
  cat("<DATABASE> ", dbplyr::escape_ansi(x$database), dbplyr::escape_ansi(x$schema), ".", dbplyr::escape_ansi(x$table))
}

#' @export
as.sql.dbplyr_database <- function(x) {
  dbplyr::ident_q(paste0(x$database, ".", x$schema, ".", x$table))
}

is_whole_number <- function(x, tol = .Machine$double.eps^0.5){
  min(abs(c(x%%1, x%%1-1))) < tol
}
