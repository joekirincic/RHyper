#' Hyper results class.
#'
#' @keywords internal
#' @import DBI
#' @import methods
#' @export
setClass(
  "HyperResult",
  contains = "DBIResult",
  slots = list(ptr = "externalptr")
)

#' Retrieve records from Hyper query
#' @export
setMethod("dbGetRowsAffected", "HyperResult", function(res, ...) {
  #TODO: Actually fill this out.
  NA_real_
})

#' Retrieve records from Hyper query
#' @export
setMethod("dbFetch", "HyperResult", function(res, n = -1, ...) {

  valid_n <- is_valid_n(n)

  if(!valid_n){
    stop("`n` must be a single whole number >= -1.")
  }

  out <- fetch_rows(res_ = res@ptr, n_ = n) %>% as.data.frame(stringsAsFactors = FALSE)

  return(out)

})

#' @export
setMethod("dbClearResult", "HyperResult", function(res, ...) {

  clear_result2(res@ptr)

  return(invisible(TRUE))

})

#' @export
setMethod("dbHasCompleted", "HyperResult", function(res, ...) {

  out <- has_completed2(res@ptr)

  return(out)

})

#' @export
setMethod("dbIsValid", "HyperResult", function(dbObj, ...){
  is_valid_result(dbObj@ptr)
})


is_valid_n <- function(x){

  if(length(x) != 1L){
    return(FALSE)
  }

  is_na <- is.na(x)
  is_less_than_negative_one <- x < -1
  is_not_whole_number <- !is_whole_number(x)

  if(is_na | is_less_than_negative_one | is_not_whole_number){
    return(FALSE)
  }

  return(TRUE)

}
