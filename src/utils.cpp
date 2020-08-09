
#include "hyperapi/hyperapi.hpp"
#include <Rcpp.h>
#include <string>
#include <vector>
#include "rhyper_types.h"
#include <R.h>
#include <Rinternals.h>



// [[Rcpp::export]]
Rcpp::CharacterVector hyper_quote_string(SEXP x){

  std::vector<std::string> x_ = Rcpp::as<std::vector<std::string>>(x);
  for(auto i = x_.begin(); i != x_.end(); i++){
    std::string quoted = hyperapi::Name(*i).toString();
    *i = quoted;
  }
  return Rcpp::wrap(x_);

}

// [[Rcpp::export]]
Rcpp::CharacterVector list_tables(SEXP conn){

  Rcpp::XPtr<HyperConnectionPtr> hc(conn);
  const hyperapi::Catalog& catalog = hc->get()->getCatalog();
  std::vector<std::string> tables;
  for(auto& schema: catalog.getSchemaNames()){
    for(auto& table: catalog.getTableNames(schema)){
      std::string table_name = table.getName().getUnescaped();
      tables.push_back(table_name);
    }
  }
  return Rcpp::wrap(tables);
}

// [[Rcpp::export]]
SEXP is_null_pointer(SEXP conn){
  int res_ = !R_ExternalPtrAddr(conn);
  return Rf_ScalarLogical(res_);
}


// // [[Rcpp::export]]
// SEXP table_exists(SEXP conn, SEXP name){
//   std::string name_ = Rcpp::as<std::string>(name);
//   Rcpp::XPtr<HyperConnectionPtr> hc(conn);
//   const hyperapi::Catalog& catalog = hc->get()->getCatalog();
//   const hyperapi::TableName& name_escaped(name_);
//   bool out = catalog.hasTable(name_escaped);
//   Rcpp::Rcout << "name_escaped: " << name_escaped.toString() << std::endl;
//   Rcpp::Rcout << "hasTable evaluates to " << out << std::endl;
//   return Rcpp::wrap(out);
// }


