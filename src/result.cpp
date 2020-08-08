#include "hyperapi/hyperapi.hpp"
#include <Rcpp.h>
#include <string>
#include <list>
#include <forward_list>
#include <vector>
#include <tuple>
#include <memory>
#include <fstream>
#include <iostream>
#include "rhyper_types.h"

typedef std::shared_ptr<hyperapi::HyperProcess> HyperProcessPtr;
typedef std::shared_ptr<hyperapi::Connection> HyperConnectionPtr;
typedef std::unique_ptr<hyperapi::Result> ResultPtr;
typedef std::shared_ptr<hyperapi::ResultIterator> ResultIteratorPtr;

// template <class T>
// std::string to_string(const T& val){
//   std::ostringstream sout;
//   sout.imbue(std::locale::classic());
//   sout << val;
//   return sout.str();
// }

// [[Rcpp::export]]
SEXP create_result(SEXP conn_, SEXP statement_) {
  Rcpp::XPtr<HyperConnectionPtr> con(conn_);
  std::string statement = Rcpp::as<std::string>(statement_);
  ResultPtr* res = new ResultPtr(new hyperapi::Result);
  **res = std::move((*con)->executeQuery(statement));
  // hyperapi::Result res = hc->get()->executeQuery(statement_);
  // hyperapi::Result* res_ = new hyperapi::Result;
  // *res_ = std::move(res);
  return Rcpp::XPtr<ResultPtr>(res, true);
}

// [[Rcpp::export]]
SEXP create_result_iterator(SEXP res_) {
  Rcpp::XPtr<ResultPtr> res(res_);
  ResultIteratorPtr* iter = new ResultIteratorPtr(new hyperapi::ResultIterator((**res), hyperapi::IteratorBeginTag()));
  ResultIteratorPtr* iterEnd = new ResultIteratorPtr(new hyperapi::ResultIterator((**res), hyperapi::IteratorEndTag()));
  auto out = Rcpp::List::create(
    Rcpp::Named("iterator") = Rcpp::XPtr<ResultIteratorPtr>(iter, true),
    Rcpp::Named("iteratorEnd") = Rcpp::XPtr<ResultIteratorPtr>(iterEnd, true)
  );
  return out;
}

// // [[Rcpp::export]]
// Rcpp::List fetch_all(SEXP res) {
//
//   Rcpp::XPtr<hyperapi::Result> hr(res);
//   int ncols = hr->getSchema().getColumnCount();
//   std::forward_list<std::vector<std::string>> data;
//
//   Rcpp::CharacterVector cols;
//   Rcpp::CharacterVector col_types;
//
//   for(auto col: hr->getSchema().getColumns()){
//     Rcpp::String col_name = Rcpp::wrap(col.getName().getUnescaped());
//     Rcpp::String col_type = Rcpp::wrap(col.getType().toString());
//     cols.push_back(col_name);
//     col_types.push_back(col_type);
//   }
//
//   for(const hyperapi::Row& row : *hr){
//     std::vector<std::string> r;
//     for(const hyperapi::Value& val: row){
//       r.push_back(to_string(val));
//     }
//     data.push_front(r);
//     //data.emplace_back(r);
//   }
//
//   // Rcpp::Rcout << "The list is " << data.size() << " rows deep." << std::endl;
//
//   if(data.empty()){
//     std::vector<std::string> r;
//     for(int i = 0; i < ncols; i++){
//       r.push_back("NULL");
//     }
//     data.push_front(r);
//   }
//
//   Rcpp::List out = Rcpp::wrap(data);
//
//   Rcpp::Environment purrr_pkg = Rcpp::Environment::namespace_env("purrr");
//   Rcpp::Environment readr_pkg = Rcpp::Environment::namespace_env("readr");
//   Rcpp::Environment hyprflex_pkg = Rcpp::Environment::namespace_env("hyprflex");
//   Rcpp::Function transpose_fn = purrr_pkg["transpose"];
//   Rcpp::Function flatten_chr_fn = purrr_pkg["flatten_chr"];
//   Rcpp::Function map_df_fn = purrr_pkg["map_df"];
//   Rcpp::Function map2_df_fn = purrr_pkg["map2_df"];
//   // Rcpp::Function parse_guess_fn = readr_pkg["parse_guess"];
//   Rcpp::Function hyper_to_r_fn = hyprflex_pkg["hyper_to_r"];
//
//   out = transpose_fn(out);
//   out.names() = cols;
//   out = map_df_fn(out, flatten_chr_fn);
//   out = map2_df_fn(out, col_types, hyper_to_r_fn);
//
//   return out;
// }

// // [[Rcpp::export]]
// Rcpp::List fetch_n(SEXP res, SEXP n) {
//
//   Rcpp::XPtr<hyperapi::Result> hr(res);
//   int n_ = Rcpp::as<int>(n);
//   int ncols = hr->getSchema().getColumnCount();
//   std::list<std::vector<std::string>> data;
//
//   Rcpp::CharacterVector cols;
//   Rcpp::CharacterVector col_types;
//
//   for(auto col: hr->getSchema().getColumns()){
//     Rcpp::String col_name = Rcpp::wrap(col.getName().getUnescaped());
//     Rcpp::String col_type = Rcpp::wrap(col.getType().toString());
//     cols.push_back(col_name);
//     col_types.push_back(col_type);
//   }
//
//   int i = 0;
//
//   for(const hyperapi::Row& row : *hr){
//     if(i == n_){ break; }
//     std::vector<std::string> r;
//     for(const hyperapi::Value& val: row){
//       r.push_back(to_string(val));
//     }
//     data.emplace_back(r);
//     i++;
//   }
//
//   // Rcpp::Rcout << "The list is " << data.size() << " rows deep." << std::endl;
//
//   if(data.size() == 0){
//     std::vector<std::string> r;
//     for(int i = 0; i < ncols; i++){
//       r.push_back("NULL");
//     }
//     data.emplace_back(r);
//   }
//
//   Rcpp::List out = Rcpp::wrap(data);
//
//   Rcpp::Environment purrr_pkg = Rcpp::Environment::namespace_env("purrr");
//   Rcpp::Environment readr_pkg = Rcpp::Environment::namespace_env("readr");
//   Rcpp::Environment hyprflex_pkg = Rcpp::Environment::namespace_env("hyprflex");
//   Rcpp::Function transpose_fn = purrr_pkg["transpose"];
//   Rcpp::Function flatten_chr_fn = purrr_pkg["flatten_chr"];
//   Rcpp::Function map_df_fn = purrr_pkg["map_df"];
//   Rcpp::Function map2_df_fn = purrr_pkg["map2_df"];
//   // Rcpp::Function parse_guess_fn = readr_pkg["parse_guess"];
//   Rcpp::Function hyper_to_r_fn = hyprflex_pkg["hyper_to_r"];
//
//   out = transpose_fn(out);
//   out.names() = cols;
//   out = map_df_fn(out, flatten_chr_fn);
//   out = map2_df_fn(out, col_types, hyper_to_r_fn);
//
//   return out;
// }

// [[Rcpp::export]]
void clear_result(SEXP res_){
  Rcpp::XPtr<ResultPtr> res(res_);
  res->get()->close();
}

// [[Rcpp::export]]
SEXP has_completed(SEXP res_){

  Rcpp::XPtr<ResultPtr> res(res_);
  bool is_open = res->get()->isOpen();
  return Rcpp::wrap(!is_open);

}
