#include "hyperapi/hyperapi.hpp"
#include "result.h"
#include "connection.h"
#include <Rcpp.h>

typedef std::unique_ptr<RHyper::connection> conn_ptr;
typedef std::shared_ptr<RHyper::result> result_ptr;
typedef std::vector<std::unique_ptr<RHyper::base_column>> colset_t;

colset_t RHyper::result::infer_colset(){
  auto schema = res_ptr->getSchema();
  colset_t out;

  for(int j = 0; j < schema.getColumnCount(); j++){
    auto t = schema.getColumn(j).getType().getTag();
    // Rcpp::Rcout << schema.getColumn(j).getType().toString() << std::endl;
    switch(t){
    case hyperapi::TypeTag::Int:
    {
      auto col = std::unique_ptr<RHyper::base_column>(new RHyper::integer_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Bool:
    {
      auto col = std::unique_ptr<RHyper::base_column>(new RHyper::bool_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Numeric:
    {
      auto col = std::unique_ptr<RHyper::base_column>(new RHyper::numeric_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::BigInt:
    case hyperapi::TypeTag::Double:
    {
      auto col = std::unique_ptr<RHyper::base_column>(new RHyper::double_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Text:
    {
      auto col = std::unique_ptr<RHyper::base_column>(new RHyper::string_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Date:
    {
      auto col = std::unique_ptr<RHyper::date_column>(new RHyper::date_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Time:
    {
      auto col = std::unique_ptr<RHyper::time_column>(new RHyper::time_column());
      out.push_back(std::move(col));
      break;
    }
    case hyperapi::TypeTag::Timestamp:
    case hyperapi::TypeTag::TimestampTZ:
    {
      auto col = std::unique_ptr<RHyper::timestamp2_column>(new RHyper::timestamp2_column());
      out.push_back(std::move(col));
      break;
    }
    default:
    {
      Rcpp::stop("Unsupported type.");
    }
    };
  }
  return out;
};

std::vector<std::string> RHyper::result::get_column_names(){
  auto schema = res_ptr->getSchema();
  std::vector<std::string> out;
  for(auto col: schema.getColumns()){
    out.push_back(col.getName().getUnescaped());
  }
  return out;
}

// [[Rcpp::export]]
SEXP create_result2(SEXP conn_, SEXP statement_){
  auto conn = Rcpp::XPtr<conn_ptr>(conn_).get();
  std::string statement = Rcpp::as<std::string>(statement_);
  result_ptr* out = new result_ptr(new RHyper::result());
  *out = conn->get()->execute_query(statement);
  return Rcpp::XPtr<result_ptr>(out, true);
}

// [[Rcpp::export]]
void clear_result2(SEXP res_){
  auto res = Rcpp::XPtr<result_ptr>(res_);
  res.release();
}

// [[Rcpp::export]]
Rcpp::List fetch_rows(SEXP res_, Rcpp::Nullable<int> n_ = R_NilValue){
  auto res = Rcpp::XPtr<result_ptr>(res_);
  if(n_.isNull()){
    auto out = res->get()->fetch();
    return out;
  }else{
    int n = Rcpp::as<int>(n_);
    auto out = res->get()->fetch(n);
    return out;
  }
}

// [[Rcpp::export]]
SEXP has_completed2(SEXP res_){

  auto res = Rcpp::XPtr<result_ptr>(res_).get()->get();
  bool out = res->is_open();
  return Rcpp::wrap(!out);

}

// [[Rcpp::export]]
bool is_valid_result(SEXP res_){
  auto res = Rcpp::XPtr<result_ptr>(res_).get();
  if(!res){
    return false;
  }
  return res->get()->check_validity();
}
