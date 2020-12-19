
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
    ptr = "externalptr",
    bigint = "character"
  )
)

#' Send a query to Hyper.
#'
#' @export
setMethod("dbSendQuery", "HyperConnection", function(conn, statement, ...) {

  result_ptr <- create_result2(conn = conn@ptr, statement = statement)

  res <- new("HyperResult", ptr = result_ptr, ...)

  return(res)
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

  disconnect(conn@ptr)

  return(invisible(TRUE))
})



#' @export
setMethod("dbListTables", "HyperConnection", function(conn, ...){

  tables <- list_tables(conn@ptr)

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
setMethod("dbWriteTable", c("HyperConnection", "character", "data.frame"), function(conn, name, value, ..., row.names = FALSE, overwrite = FALSE, append = FALSE, temporary = FALSE){

  name_escaped <- DBI::dbQuoteIdentifier(conn, name)

  create_statement <- DBI::sqlCreateTable(
    conn,
    name_escaped,
    fields = DBI::dbDataType(conn, value),
    row.names = FALSE
  )

  execute_command(conn@ptr, create_statement)

  escaped_value <- DBI::sqlData(con = conn, name = name_escaped, row.names = FALSE)

  insert_statement <- DBI::sqlAppendTable(con = conn, table = name_escaped, values = escaped_value, row.names = FALSE)

  execute_command(conn@ptr, insert_statement)

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

  execute_command(conn@ptr, statement)

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

  attach_database(conn@ptr, db_file)

})

#' Remove a Hyper database from the current Hyper process.
#'
#' @export
setMethod("dbDetachDatabase", c("HyperConnection", "character"), function(conn, name, ...) {

  detach_database(conn@ptr, name)

})

#' @export
setMethod("dbIsValid", "HyperConnection", function(dbObj, ...) {
  is_valid_connection(dbObj@ptr)
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




