
#' Hyper connection class.
#'
#' @export
#' @keywords internal
#' @import DBI
#' @import methods
setClass(
  "HyperConnection",
  contains = "DBIConnection",
  slots = list(
    telemetry = "logical",
    database = "environment",
    process_ptr = "externalptr",
    connection_ptr = "externalptr",
    bigint = "character"
  )
)

#' Send a query to Hyper.
#'
#' @export
setMethod("dbSendQuery", "HyperConnection", function(conn, statement, ...) {

  # release_all(conn = conn)

  result_ptr <- create_result(conn = conn@connection_ptr, statement = statement)
  result_iterator <- create_result_iterator(result_ptr)
  .RHyperSession$Result$iterator <- result_iterator

  res <- new("HyperResult", result_ptr = result_ptr, ...)

  # conn@result$active <- res

  res
})

#' Show details about a Hyper Connection.
#'
#' @param HyperConnection
#'
#' @export
setMethod("show", "HyperConnection", function(object){
  cat("<HyperConnection>\n")
})

#' Terminate a Hyper Connection.
#'
#' @param HyperConnect
#'
#' @export
setMethod("dbDisconnect", "HyperConnection", function(conn){

  disconnect(conn@connection_ptr)
  terminate(conn@process_ptr)

  return(invisible(TRUE))

})



#' @export
setMethod("dbListTables", "HyperConnection", function(conn, ...){

  tables <- list_tables(conn@connection_ptr)

  return(tables)

})

#' @export
setMethod("dbExistsTable", "HyperConnection", function(conn, name, ...){

  if(is.null(name)){
    stop("`name` cannot be NULL")
  }
  out <- name %in% dbListTables(conn)

  return(out)

})

#' @export
setMethod("dbDataType", "HyperConnection", function(dbObj, obj, ...){
  if (is.data.frame(obj)) return(vapply(obj, dbDataType, "", dbObj = dbObj))
  get_data_type(obj)
})

get_data_type <- function(obj) {
  if (is.factor(obj)) return("TEXT")
  if (inherits(obj, "POSIXt")) return("TIMESTAMPTZ")
  if (inherits(obj, "Date")) return("DATE")
  if (inherits(obj, "difftime")) return("TIME")
  if (inherits(obj, "integer64")) return("BIGINT")
  switch(typeof(obj),
         integer = "INTEGER",
         double = "DOUBLE PRECISION",
         character = "TEXT",
         logical = "BOOLEAN",
         list = "BYTEA",
         stop("Unsupported type", call. = FALSE)
  )
}

#' @export
setMethod("dbWriteTable", c("HyperConnection", "character", "data.frame"), function(conn, name, value, ...){

  name_escaped <- DBI::dbQuoteIdentifier(conn, name)

  create_statement <- DBI::sqlCreateTable(
    conn,
    name_escaped,
    fields = DBI::dbDataType(conn, value)
  )

  query_execute(conn@connection_ptr, create_statement)

  insert_statement <- DBI::sqlAppendTable(con = conn, table = name_escaped, values = value)

  query_execute(conn@connection_ptr, insert_statement)

  return(TRUE)

})

#' @export
setMethod("dbRemoveTable", c("HyperConnection", "character"), function(conn, name, ...){

  name_escaped <- DBI::dbQuoteIdentifier(conn, name)

  DBI::dbExecute(conn, paste0("DROP TABLE IF EXISTS", name_escaped))

  invisible(TRUE)

})

#' @export
setMethod("dbExecute", c("HyperConnection", "character"), function(conn, statement, ...){

  query_execute(conn@connection_ptr, statement)

  invisible(TRUE)

})

#' @export
setGeneric(
  "dbAttachDatabase",
  def = function(conn, db_file, name, ...) standardGeneric("dbAttachDatabase")
)

#' @export
setGeneric(
  "dbDetachDatabase",
  def = function(conn, name, ...) standardGeneric("dbDetachDatabase")
)


#' Add a Hyper database to the current Hyper process.
#'
#' @export
setMethod("dbAttachDatabase", c("HyperConnection", "character", "character"), function(conn, db_file, name = NA_character_, ...) {

  nm <- if(is.na(name)){ fs::path_ext_remove(fs::path_file(db_file)) }else{ name }

  attach_database(conn@connection_ptr, db_file)

  conn@database[[nm]] <- db_file

})

#' Remove a Hyper database from the current Hyper process.
#'
#' @export
setMethod("dbDetachDatabase", c("HyperConnection", "character"), function(conn, name, ...) {

  # db_name <- fs::path_ext_remove(fs::path_file(db_file))

  detach_database(conn@connection_ptr, name)

  rm(list = name, envir = conn@database)

})

#' @export
setMethod("dbGetInfo", "HyperConnection", function(dbObj, ...){
  #TODO: figure out how to get hyperd version no. and hyperapi version no.
  list(
    db.version = NA_character_,
    dbname = NA_character_,
    username = NA_character_,
    host = NA_character_,
    port = NA_character_,
    date_style = NA_character_
  )
})




