

#' Driver for Hyper database.
#'
#' @keywords internal
#' @export
#' @import DBI
#' @import methods
setClass("HyperDriver", contains = "DBIDriver")

#' @export
#' @rdname HyperDriver-class
setMethod("dbUnloadDriver", "HyperDriver", function(drv, ...) {
  TRUE
})

#' @param HyperDriver
#'
#' @export
#'
#' @rdname HyperDriver-class
setMethod("show", "HyperDriver", function(object) {
  cat("<HyperDriver>\n")
})

#' @export
#' @rdname HyperDriver-class
Hyper <- function() {
  new("HyperDriver")
}

#' @param drv An object created by \code{Hyper()}
#' @rdname HyperDriver-class
#' @export
setMethod("dbConnect", "HyperDriver", function(drv, db = NULL, bigint = "numeric", ...) {

  if(!is.null(db)){
    db <- RHyper:::sanitize_connection_info(db)
  }

  conn_ptr <- connect(unname(db), names(db))

  out <- new("HyperConnection", ptr = conn_ptr, bigint = bigint, ...)

  return(out)

})

#' @export
#' @rdname HyperDriver-class
setMethod("dbGetInfo", "HyperDriver", function(dbObj, ...){
  #TODO: figure out how to get hyperd version no. and hyperapi version no.
  list(
    driver.version = NA_character_,
    client.version = NA_character_
  )
})

sanitize_connection_info <- function(path){

  if(is.null(names(path))){
    nm <- rep(NA_character_, length(path))
  }else{
    nm <- names(path)
  }

  iterables <- list(db_path = unname(path), is_aliased = rlang::have_name(path), db_alias = nm)

  func <- function(db_path, is_aliased, db_alias){
    if(is_aliased){
      return(db_alias)
    }
    tools::file_path_sans_ext(basename(db_path))
  }

  aliases <- purrr::pmap_chr(.l = iterables, .f = func)

  return(rlang::set_names(path, aliases))

}
