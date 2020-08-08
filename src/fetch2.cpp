#include "hyperapi/hyperapi.hpp"
#include <forward_list>
#include <string>
#include <vector>
#include <memory>
#include <Rcpp.h>
#include "rhyper_types.h"

// [[Rcpp::export]]
SEXP fetch_all(SEXP res_){

  Rcpp::Environment dplyr_pkg = Rcpp::Environment::namespace_env("dplyr");
  Rcpp::Function bind_rows_fn = dplyr_pkg["bind_rows"];

  Rcpp::XPtr<ResultPtr> res(res_);

  Rcpp::CharacterVector col_names;
  std::vector<hyperapi::TypeTag> col_types;
  std::list<Rcpp::List> data;

  for(auto col: res->get()->getSchema().getColumns()){
    col_names.push_back(col.getName().getUnescaped());
    col_types.push_back(col.getType().getTag());
  }

  auto n_col = col_names.size();

  if(!res->get()->isOpen()){
    Rcpp::List r;
    for(int j = 0; j < n_col; j++){
      auto ht = col_types[j];

      switch(ht){
      case hyperapi::TypeTag::BigInt:
      case hyperapi::TypeTag::Numeric:
      {
        Rcpp::NumericVector out;
        r.push_back(out);
        break;
      } // end BigInt, Numeric
      case hyperapi::TypeTag::Bool:
      {
        Rcpp::LogicalVector out;
        r.push_back(out);
        break;
      } // end Bool
      case hyperapi::TypeTag::Char:
      case hyperapi::TypeTag::Text:
      case hyperapi::TypeTag::Time:
      {
        Rcpp::CharacterVector out;
        r.push_back(out);
        break;
      } // end Char, Text, and Time
      case hyperapi::TypeTag::SmallInt:
      case hyperapi::TypeTag::Int:
      {
        Rcpp::IntegerVector out;
        r.push_back(out);
        break;
      } // end SmallInt, Int
      case hyperapi::TypeTag::Double:
      {
        Rcpp::DoubleVector out;
        r.push_back(out);
        break;
      } // end Double
      case hyperapi::TypeTag::Date:
      {
        Rcpp::DateVector out(0);
        r.push_back(out);
        break;
      } // end Date
      case hyperapi::TypeTag::Timestamp:
      {
        Rcpp::DatetimeVector out(0);
        r.push_back(out);
        break;
      } // end Timestamp
      default:
      {
        Rcpp::stop("Unsupported type.");
      }
      } // end switch

    }
    data.emplace_back(r);
    Rcpp::List out = Rcpp::wrap(data);
    out.names() = col_names;
    out = bind_rows_fn(out);
    return out;
  }

  hyperapi::Chunks chunks((**res));
  auto first = begin(chunks);
  auto last = end(chunks);

  while(first != last){

    auto n_row = first->getRowCount();
    auto n_col = col_types.size();

    std::vector<Rcpp::RObject> df(n_col);

    for(int j = 0; j < n_col; j++){
      auto ht = col_types[j];
      switch(ht){
      case hyperapi::TypeTag::BigInt:
      case hyperapi::TypeTag::Numeric:
      {

        Rcpp::NumericVector out(n_row);
        out.fill(NA_REAL);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            out[i] = v.get<double>();
          }
        }
        df.at(j) = out;
        break;
      } // end BigInt, Numeric
      case hyperapi::TypeTag::Bool:
      {
        Rcpp::LogicalVector out(n_row);
        out.fill(NA_LOGICAL);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            out[i] = v.get<bool>();
          }
        }
        df.at(j) = out;
        break;
      } // end Bool
      case hyperapi::TypeTag::Char:
      case hyperapi::TypeTag::Text:
      case hyperapi::TypeTag::Time:
      {
        Rcpp::CharacterVector out(n_row);
        out.fill(NA_STRING);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            out[i] = v.get<std::string>();
          }
        }
        df.at(j) = out;
        break;
      } // end Char, Text, and Time
      case hyperapi::TypeTag::SmallInt:
      case hyperapi::TypeTag::Int:
      {
        Rcpp::IntegerVector out(n_row);
        out.fill(NA_INTEGER);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            out[i] = v.get<int>();
          }
        }
        df.at(j) = out;
        break;
      } // end SmallInt, Int
      case hyperapi::TypeTag::Double:
      {
        Rcpp::DoubleVector out(n_row);
        out.fill(NA_REAL);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            out[i] = v.get<double>();
          }
        }
        df.at(j) = out;
        break;
      } // end Double
      case hyperapi::TypeTag::Date:
      {
        Rcpp::DateVector out(n_row);
        out.fill(NA_REAL);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            hyperapi::Date d = v.get<hyperapi::Date>();
            out[i] = Rcpp::Date(d.getYear(), d.getMonth(), d.getDay());
          }
        }
        df.at(j) = out;
        break;
      } // end Date
      case hyperapi::TypeTag::Timestamp:
      {
        Rcpp::DatetimeVector out(n_row);
        out.fill(NA_REAL);
        for(int i = 0; i < n_row; i++){
          const auto& v = first->getRowAt(i).get<>(j);
          if(!v.isNull()){
            hyperapi::Timestamp dt = v.get<hyperapi::Timestamp>();
            out[i] = Rcpp::Datetime(dt.toString());
          }
        }
        df.at(j) = out;
        break;
      } // end Timestamp
      default:
      {
        Rcpp::stop("Unsupported type.");
      }
      } // end switch
    }
    Rcpp::List lst = Rcpp::wrap(df);
    lst.names() = col_names;
    data.push_back(lst);
    ++first;
    Rcpp::checkUserInterrupt();
  }

  SEXP out = bind_rows_fn(data);
  return out;

}

// [[Rcpp::export]]
SEXP fetch_n2(SEXP res_, SEXP iter_, SEXP iter_end_, SEXP n_){

  Rcpp::Environment purrr_pkg = Rcpp::Environment::namespace_env("purrr");
  Rcpp::Environment base_pkg = Rcpp::Environment::namespace_env("base");
  Rcpp::Function map_dfc_fn = purrr_pkg["map_dfc"];
  Rcpp::Function unlist_fn = base_pkg["unlist"];

  Rcpp::XPtr<ResultPtr> res(res_);
  Rcpp::XPtr<ResultIteratorPtr> curr(iter_);
  Rcpp::XPtr<ResultIteratorPtr> last(iter_end_);
  auto n = Rcpp::as<int>(n_);

  Rcpp::CharacterVector col_names;
  std::vector<hyperapi::TypeTag> col_types;

  for(auto col: res->get()->getSchema().getColumns()){
    col_names.push_back(col.getName().getUnescaped());
    col_types.push_back(col.getType().getTag());
  }

  auto n_col = col_names.size();
  auto rows_fetched = 0;

  std::vector<std::list<Rcpp::RObject>> data(n_col);

  // The fetch_* functions should always return a data.frame.
  //
  if(!res->get()->isOpen()){
    for(int j = 0; j < n_col; j++){
      auto ht = col_types[j];

      switch(ht){
      case hyperapi::TypeTag::BigInt:
      case hyperapi::TypeTag::Numeric:
      {
        Rcpp::NumericVector out(1);
        out.fill(NA_REAL);
        data.at(j).push_back(out);
        break;
      } // end BigInt, Numeric
      case hyperapi::TypeTag::Bool:
      {
        Rcpp::LogicalVector out(1);
        out.fill(NA_LOGICAL);
        data.at(j).push_back(out);
        break;
      } // end Bool
      case hyperapi::TypeTag::Char:
      case hyperapi::TypeTag::Text:
      case hyperapi::TypeTag::Time:
      {
        Rcpp::CharacterVector out(1);
        out.fill(NA_STRING);
        data.at(j).push_back(out);
        break;
      } // end Char, Text, and Time
      case hyperapi::TypeTag::SmallInt:
      case hyperapi::TypeTag::Int:
      {
        Rcpp::IntegerVector out(1);
        out.fill(NA_INTEGER);
        data.at(j).push_back(out);
        break;
      } // end SmallInt, Int
      case hyperapi::TypeTag::Double:
      {
        Rcpp::DoubleVector out(1);
        out.fill(NA_REAL);
        data.at(j).push_back(out);
        break;
      } // end Double
      case hyperapi::TypeTag::Date:
      {
        Rcpp::DateVector out(1);
        out.fill(NA_REAL);
        data.at(j).push_back(out);
        break;
      } // end Date
      case hyperapi::TypeTag::Timestamp:
      {
        Rcpp::DatetimeVector out(1);
        out.fill(NA_REAL);
        data.at(j).push_back(out);
        break;
      } // end Timestamp
      default:
      {
        Rcpp::stop("Unsupported type.");
      }
      } // end switch

    }
    Rcpp::List out = Rcpp::wrap(data);
    out.names() = col_names;
    out = map_dfc_fn(out, unlist_fn);
    return out;
  }

  while(**curr != **last){
    const hyperapi::Row& row = ***curr;
    std::vector<Rcpp::RObject> r(n_col);
    for(int j = 0; j < n_col; j++){
      auto ht = col_types[j];

      switch(ht){
      case hyperapi::TypeTag::BigInt:
      case hyperapi::TypeTag::Numeric:
      {
        Rcpp::NumericVector out(1);
        out.fill(NA_REAL);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          out[0] = v.get<double>();
          data.at(j).push_back(out);
        }
        break;
      } // end BigInt, Numeric
      case hyperapi::TypeTag::Bool:
      {
        Rcpp::LogicalVector out(1);
        out.fill(NA_LOGICAL);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          out[0] = v.get<bool>();
          data.at(j).push_back(out);
        }
        break;
      } // end Bool
      case hyperapi::TypeTag::Char:
      case hyperapi::TypeTag::Text:
      case hyperapi::TypeTag::Time:
      {
        Rcpp::CharacterVector out(1);
        out.fill(NA_STRING);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          out[0] = v.get<std::string>();
          data.at(j).push_back(out);
        }
        break;
      } // end Char, Text, and Time
      case hyperapi::TypeTag::SmallInt:
      case hyperapi::TypeTag::Int:
      {
        Rcpp::IntegerVector out(1);
        out.fill(NA_INTEGER);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          out[0] = v.get<int>();
          data.at(j).push_back(out);
        }
        break;
      } // end SmallInt, Int
      case hyperapi::TypeTag::Double:
      {
        Rcpp::DoubleVector out(1);
        out.fill(NA_REAL);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          out[0] = v.get<double>();
          data.at(j).push_back(out);
        }
        break;
      } // end Double
      case hyperapi::TypeTag::Date:
      {
        Rcpp::DateVector out(1);
        out.fill(NA_REAL);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          hyperapi::Date d = v.get<hyperapi::Date>();
          out[0] = Rcpp::Date(d.getYear(), d.getMonth(), d.getDay());
          data.at(j).push_back(out);
        }
        break;
      } // end Date
      case hyperapi::TypeTag::Timestamp:
      {
        Rcpp::DatetimeVector out(1);
        out.fill(NA_REAL);
        const hyperapi::Value& v = row.get<>(j);
        if(!v.isNull()){
          hyperapi::Timestamp dt = v.get<hyperapi::Timestamp>();
          out[0] = Rcpp::Datetime(dt.toString());
          data.at(j).push_back(out);
        }
        break;
      } // end Timestamp
      default:
      {
        Rcpp::stop("Unsupported type.");
      }
      } // end switch
    }
    ++(**curr);
    rows_fetched += 1;
    if(rows_fetched >= n){ break; }
  }

  Rcpp::List out = Rcpp::wrap(data);
  out.names() = col_names;
  out = map_dfc_fn(out, unlist_fn);

  return out;
}
