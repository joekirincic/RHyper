# Generated by using Rcpp::compileAttributes() -> do not edit by hand
# Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

connect <- function(database_ = NULL, aliases_ = NULL) {
    .Call(`_RHyper_connect`, database_, aliases_)
}

disconnect <- function(connection_ptr) {
    invisible(.Call(`_RHyper_disconnect`, connection_ptr))
}

execute_command <- function(conn_, statement_) {
    invisible(.Call(`_RHyper_execute_command`, conn_, statement_))
}

is_valid_connection <- function(conn_) {
    .Call(`_RHyper_is_valid_connection`, conn_)
}

file_name_impl <- function(path_) {
    .Call(`_RHyper_file_name_impl`, path_)
}

create_result2 <- function(conn_, statement_) {
    .Call(`_RHyper_create_result2`, conn_, statement_)
}

clear_result2 <- function(res_) {
    invisible(.Call(`_RHyper_clear_result2`, res_))
}

fetch_rows <- function(res_, n_ = NULL) {
    .Call(`_RHyper_fetch_rows`, res_, n_)
}

has_completed2 <- function(res_) {
    .Call(`_RHyper_has_completed2`, res_)
}

is_valid_result <- function(res_) {
    .Call(`_RHyper_is_valid_result`, res_)
}

