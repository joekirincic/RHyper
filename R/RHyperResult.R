#' Hyper results class.
#'
#' @keywords internal
#' @import DBI
#' @import methods
#' @export
setClass(
  "HyperResult",
  contains = "DBIResult",
  slots = list(result_ptr = "externalptr")
)

#' Retrieve records from Hyper query
#' @export
setMethod("dbGetRowsAffected", "HyperResult", function(res, ...) {
  NA_real_
})

#' Retrieve records from Hyper query
#' @export
setMethod("dbFetch", "HyperResult", function(res, n = -1, ...) {

  if(n == -1){
    out <- fetch_all(res = res@result_ptr)
  }else{
    out <- dplyr::as_tibble(fetch_n2(res@result_ptr, .RHyperSession$Result$iterator$iterator, .RHyperSession$Result$iterator$iteratorEnd, n))
  }

  # out <- dplyr::as_tibble(fetch_n2(res@result_ptr, .RHyperSession$Result$iterator$iterator, .RHyperSession$Result$iterator$iteratorEnd, n))

  return(out)

})

#' @export
setMethod("dbClearResult", "HyperResult", function(res, ...) {

  clear_result(res@result_ptr)

  return(invisible(TRUE))
})

#' @export
setMethod("dbHasCompleted", "HyperResult", function(res, ...) {

  out <- has_completed(res@result_ptr)

  return(out)

})

#' @export
hyper_to_r <- function(column, column_type){

  na_vals <- c("", "NA", "NULL")

  switch (column_type,
    "DATE" = readr::parse_date(column, na = na_vals),
    "TEXT" = readr::parse_character(column, na = na_vals),
    "TIMESTAMP" = readr::parse_datetime(column, na = na_vals),
    "TIMESTAMPTZ" = readr::parse_datetime(column, na = na_vals),
    "TIME" = readr::parse_time(column, na = na_vals),
    "BIGINT" = readr::parse_number(column, na = na_vals),
    "INTEGER" = readr::parse_integer(column, na = na_vals),
    "DOUBLE PRECISION" = readr::parse_double(column, na = na_vals),
    "BOOLEAN" = readr::parse_logical(column, na = na_vals),
    stop("Unsupported type", call. = FALSE)
  )

}

#' @export
#' @keywords internal
release_all <- function(conn){
  # objs <- mget(ls(), envir = globalenv()) %>% purrr::keep(.p = ~isS4(.x)) %>% purrr::keep(.p = ~(class(.x) == "HyperResult"))
  # if(length(objs) >= 1){
  #   message("Active HyperResult(s) found. Closing others before execution...")
  #   purrr::walk(objs, ~DBI::dbClearResult(.x))
  # }
  objs <- as.list(conn@result)

  has_open_results <- if(length(objs) == 0){ FALSE }else{ TRUE }

  if(has_open_results){
    purrr::walk(objs, ~DBI::dbClearResult(.x))
  }

}

