

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
setMethod("dbConnect", "HyperDriver", function(drv, telemetry = FALSE, database = list(), bigint = "numeric", ...) {

  # Initialize Hyper Process.
  process_ptr <- create_hyper_process(telemetry = telemetry)

  # Attach Hyper files.
  connection_ptr <- create_hyper_connection(process_ptr = process_ptr, database = database)

  db <- if(is.null(names(database))){
    db_names <- purrr::map_chr(database, ~fs::path_ext_remove(fs::path_file(.x)))
    purrr::set_names(database, db_names) %>% list2env()
  }else{
    list2env(database)
  }

  new("HyperConnection", telemetry = telemetry, database = db, process_ptr = process_ptr, connection_ptr = connection_ptr, bigint = bigint, ...)
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
