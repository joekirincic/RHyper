
#ifndef __RHYPER_COLUMN__
#define __RHYPER_COLUMN__

#include "hyperapi/hyperapi.hpp"
#include <vector>
#include <chrono>
#include <Rcpp.h>

inline time_t toUTC(std::tm& timeinfo)
{
#ifdef _WIN32
  std::time_t tt = _mkgmtime(&timeinfo);
#else
  time_t tt = timegm(&timeinfo);
#endif
  return tt;
}

inline double get_seconds_since_epoch(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
) // these are UTC values
{
  tm timeinfo1 = tm();
  timeinfo1.tm_year = year - 1900;
  timeinfo1.tm_mon = month - 1;
  timeinfo1.tm_mday = day;
  timeinfo1.tm_hour = hour;
  timeinfo1.tm_min = minute;
  timeinfo1.tm_sec = second;
  tm timeinfo = timeinfo1;
  time_t tt = toUTC(timeinfo);
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(tt);
  auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
  return seconds_since_epoch;
};


namespace RHyper {

/*
 * typdef below is no longer needed.
 */
// typedef std::variant
// <
//   hyperapi::optional<int>,
//   hyperapi::optional<int64_t>,
//   hyperapi::optional<bool>,
//   hyperapi::optional<double>,
//   hyperapi::optional<std::string>,
//   hyperapi::optional<hyperapi::Date>,
//   hyperapi::optional<hyperapi::Timestamp>
// > hyper_data;

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
  std::vector<hyperapi::optional<int>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<int>>());
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
  std::vector<hyperapi::optional<double>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<double>>());
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
  std::vector<hyperapi::optional<bool>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<bool>>());
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
  std::vector<hyperapi::optional<std::string>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<std::string>>());
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
  std::vector<hyperapi::optional<hyperapi::Date>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<hyperapi::Date>>());
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
  std::vector<hyperapi::optional<hyperapi::Timestamp>> data;
public:
  void ingest(const hyperapi::Value& v){
    if(data.size() == data.capacity()){
      growth_factor++;
      data.reserve(data.size() * growth_factor);
    }
    data.push_back(v.get<hyperapi::optional<hyperapi::Timestamp>>());
  };
  Rcpp::RObject to_sexp(){
    Rcpp::DatetimeVector out(data.size());
    for(int i = 0; i < data.size(); i++){
      if(data[i]){
        auto dt = data[i].value();
        auto seconds_since_epoch = get_seconds_since_epoch(
          dt.getDate().getYear(),
          dt.getDate().getMonth(),
          dt.getDate().getDay(),
          dt.getTime().getHour(),
          dt.getTime().getMinute(),
          dt.getTime().getSecond()
        );
        out[i] = Rcpp::Datetime(seconds_since_epoch);
        // out[i] = Rcpp::Datetime(dt.toString());
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
//     contents.push_back(x.get<hyperapi::optional<int>>());
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
//         [](hyper_data x){ return (std::get<hyperapi::optional<int>>(x)).value_or(NA_INTEGER); }
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
