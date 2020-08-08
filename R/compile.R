

cpp <- function(x){ structure(x, class = "cpp")}

print.cpp <- function(x, ...){
  out <- paste0("<CPP> ", x)
  cat(paste(strwrap(out), collapse = "\n"), "\n", sep = "")
}

escape <- function(x){ UseMethod("escape") }

escape.character <- function(x){
  cpp(x)
}

escape.cpp <- function(x){
  x
}

tuple <- function(...){
  dots <- dots_partition(...)
  cpp(
    paste0("std::tuple", "<", paste(dots$unnamed, collapse = ","), ">")
  )
}

dots_partition <- function(...){

  dots <- rlang::list2(...)

  if( is.null(names(dots)) ){
    is_named <- rep(FALSE, length(dots))
  }else{
    is_named <- names(dots) != ""
  }

  list(
    named = dots[is_named],
    unnamed = dots[!is_named]
  )

}

cpp_attributes <- function(list){
  if( length(list) == 0 ){ return("") }
  attr <- purrr::map2_chr(names(list), list, cpp_attribute)
  paste0(" ", unlist(attr), collapse = ",")
}

cpp_attribute <- function(name, value = NULL){
  if( length(value == 0)){ return( paste0(name, "=", value)) }
  if( length(value != 0)){ stop("`value` must be NULL or length 1")}
}

cpp_obj <- function(obj){
  rlang::new_function(
    rlang::exprs(... = , namespace = NULL),
    rlang::expr({

      dots <- dots_partition(...)
      attribs <- cpp_attributes(dots$named)
      children <- purrr::map_chr(dots$unnamed, escape)

      namespace_qualifier <- if(!is.null(namespace)){ paste0(namespace, "::") }
      type_qualifier <- if(length(children) >= 1){ paste0("<", paste(children, collapse = ", "), ">") }
      input_params <- if(length(dots$named) >= 1){ paste0("(", attribs, ")") }

      cpp(
        paste0(
          namespace_qualifier,
          !!paste0(obj),
          type_qualifier,
          input_params
        )
      )
    }),
    rlang::caller_env()
  )
}

build_cpp_fn <- function(name, otype, ...){
  if(is.null(name) || is.null(otype)) stop("`name` or `otype` is NULL.")
  dots <- dots_partition(...)
  fn_body <- purrr::as_vector(dots$named) %>% glue::glue_collapse(sep = "\n")
  fn <- glue::glue(.open = '|', .close = '|',
  "SEXP |name|(SEXP conn){
    |fn_body|
  }")
  return(fn)
}

fetch_boilerplate2 <- function(row_spec){

  forward_list <- cpp_obj("forward_list")

  row_assignment_chunk <- purrr::map_chr(
    seq_along(row_spec),
    .f = ~glue::glue("    v{.x}.push_front(row.get<{row_spec[[.x]]}>({.x - 1}));")
  ) %>%
    glue::glue_collapse(sep = "\n")

  column_vectors_chunk <- purrr::map_chr(seq_along(row_spec), ~forward_list(row_spec[[.x]], namespace = "std") %>% glue::glue(" v{.x};")) %>% glue::glue_collapse(sep = "\n")

  glue::glue(.open = "|", .close = "|", .sep = "\n",
             "Rcpp::DataFrame fetch_new(SEXP res){",
             "Rcpp::XPtr<hyperapi::Result> hr(res);",
             "int ncols = hr->getSchema().getColumnCount();",
             "Rcpp::CharacterVector cols;",
             "",
             "for(auto col: hr->getSchema().getColumns()){",
             "  Rcpp::String col_name = Rcpp::wrap(col.getName().getUnescaped());",
             "  cols.push_back(col_name);",
             "}",
             "",
             column_vectors_chunk,
             "",
             "for(const hyperapi::Row& row : *hr){",
             row_assignment_chunk,
             "}",
             glue::glue('Rcpp::DataFrame out = Rcpp::DataFrame::create({purrr::map_chr(seq_along(row_spec), ~glue::glue("v{.x}")) %>% paste(collapse = ", ")}, _["stringsAsFactors"] = false);'),
             "out.names() = cols;",
             "return Rcpp::wrap(out);",
             "}"
  )
}

