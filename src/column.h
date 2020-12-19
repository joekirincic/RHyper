
#ifndef __RHYPER_COLUMN__
#define __RHYPER_COLUMN__

#include "hyperapi/hyperapi.hpp"
#include <vector>
#include <variant>
#include <optional>
#include <Rcpp.h>

namespace RHyper {

typedef std::variant
<
  std::optional<int>,
  std::optional<int64_t>,
  std::optional<bool>,
  std::optional<double>,
  std::optional<std::string>,
  std::optional<hyperapi::Date>,
  std::optional<hyperapi::Timestamp>
> hyper_data;

class base_column {
public:
  base_column(){};
  base_column(base_column const &)=delete;
  base_column &operator=(base_column const &)=delete;
  base_column(base_column &&o){};
  base_column &operator=(base_column &&o){
    if (this != &o)
    {
     std::move(o);
    }
    return *this;
  };
  virtual void ingest(const hyperapi::Value& v){  Rcpp::stop("Value is of unsupported type"); };
  virtual Rcpp::RObject to_sexp(){ Rcpp::stop("Unsupported type"); };
};

class integer_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<int>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<int>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::IntegerVector out = Rcpp::no_init(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        out[i] = data[i].value();
      }else{
        out[i] = NA_INTEGER;
      }
    }
    return out;
  };
};

class double_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<double>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<double>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::DoubleVector out = Rcpp::no_init(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        out[i] = data[i].value();
      }else{
        out[i] = NA_REAL;
      }
    }
    return out;
  };
};

class bool_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<bool>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<bool>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::LogicalVector out = Rcpp::no_init(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        out[i] = data[i].value();
      }else{
        out[i] = NA_LOGICAL;
      }
    }
    return out;
  };
};

class string_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<std::string>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<std::string>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::CharacterVector out = Rcpp::no_init(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        out[i] = data[i].value();
      }else{
        out[i] = NA_STRING;
      }
    }
    return out;
  };
};

class date_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<hyperapi::Date>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<hyperapi::Date>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::DateVector out(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        auto d = data[i].value();
        out[i] = Rcpp::Date(d.getYear(), d.getMonth(), d.getDay());
      }else{
        out[i] = NA_REAL;
      }
    }
    return out;
  };
};

class timestamp_column: public base_column {
private:
  int growth_factor = 1;
  std::vector<std::optional<hyperapi::Timestamp>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<std::optional<hyperapi::Timestamp>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::DatetimeVector out(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        auto dt = data[i].value();
        out[i] = Rcpp::Datetime(dt.toString());
      }else{
        out[i] = NA_REAL;
      }
    }
    return out;
  };
};

// typedef void (column::*col_read_fn)(const hyperapi::Value);
//
// class column {
// private:
//   hyperapi::TypeTag col_type;
//   std::list<hyper_data> contents;
// public:
//   column(column const &)=delete;
//   column &operator=(column const &)=delete;
//   column(hyperapi::TypeTag t):
//     col_type(std::move(t)) {};
//   column(column &&o):
//     col_type(std::move(o.col_type)), contents(o.contents) {};
//   column &operator=(column &&o){
//     if (this != &o)
//     {
//       col_type = std::move(o.col_type);
//       contents = std::move(o.contents);
//     }
//     return *this;
//   };
//   void ingest(const hyperapi::Value& x){
//     contents.push_back(x.get<std::optional<int>>());
//   };
//   Rcpp::RObject get_r_vector(){
//     switch(col_type){
//     case hyperapi::TypeTag::Int:
//     {
//       R_xlen_t n = contents.size();
//       Rcpp::IntegerVector r_vec(n);
//       std::transform(
//         contents.begin(),
//         contents.end(),
//         r_vec.begin(),
//         [](hyper_data x){ return (std::get<std::optional<int>>(x)).value_or(NA_INTEGER); }
//       );
//       return r_vec;
//     }
//     default:
//       {
//         Rcpp::stop("Unsupported type.");
//       }
//     };
//   };
// };

}

#endif
