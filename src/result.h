
#ifndef __RHYPER_RES__
#define __RHYPER_RES__

#include "hyperapi/hyperapi.hpp"
#include <memory>
#include <Rcpp.h>
#include "column.h"

typedef std::vector<std::unique_ptr<RHyper::base_column>> colset_t;

namespace RHyper {

class result {
private:
  std::unique_ptr<hyperapi::Result> res_ptr = std::unique_ptr<hyperapi::Result>(nullptr);
  std::unique_ptr<hyperapi::ResultIterator> iter_start_ptr = std::unique_ptr<hyperapi::ResultIterator>(nullptr);
  std::unique_ptr<hyperapi::ResultIterator> iter_end_ptr = std::unique_ptr<hyperapi::ResultIterator>(nullptr);
  std::string statement;
public:
  result(){};
  result(result const &)=delete;
  result &operator=(result const &)=delete;
  result(std::unique_ptr<hyperapi::Result> &r, std::unique_ptr<hyperapi::ResultIterator> &s, std::unique_ptr<hyperapi::ResultIterator> &e, std::string sql):
    res_ptr(std::move(r)), iter_start_ptr(std::move(s)), iter_end_ptr(std::move(e)), statement(sql) {};
  result(result &&o) : res_ptr(std::move(o.res_ptr)), iter_start_ptr(std::move(o.iter_start_ptr)), iter_end_ptr(std::move(o.iter_end_ptr)), statement(std::move(o.statement)) {};
  result &operator=(result &&o){
    if (this != &o)
    {
      res_ptr = std::move(o.res_ptr);
      iter_start_ptr = std::move(o.iter_start_ptr);
      iter_end_ptr = std::move(o.iter_end_ptr);
      statement = std::move(o.statement);
    }
    return *this;
  };
  bool is_open(){ return res_ptr->isOpen(); };
  bool is_tapped(){
    bool out = *iter_start_ptr == *iter_end_ptr;
    return out;
  };
  std::string get_statement(){
    return statement;
  };
  colset_t infer_colset();
  std::vector<std::string> get_column_names();
  Rcpp::List fetch(int n = -1){
    colset_t column_set = infer_colset();
    std::vector<std::string> col_names = get_column_names();
    if(n == -1){
      while(*iter_start_ptr != *iter_end_ptr){
        const hyperapi::Row& r = **iter_start_ptr;
        for(int j = 0; j < column_set.size(); j++){
          (*column_set[j]).ingest(r.get<>(j));
        }
        ++(*iter_start_ptr);
      }
    }else{
      int i = 0;
      while(*iter_start_ptr != *iter_end_ptr){
        const hyperapi::Row& r = **iter_start_ptr;
        for(int j = 0; j < column_set.size(); j++){
          (*column_set[j]).ingest(r.get<>(j));
        }
        i++;
        ++(*iter_start_ptr);
        if(i >= n){ break; }
      }
    }
    std::vector<Rcpp::RObject> tmp;
    for(int j = 0; j < column_set.size(); j++){
      tmp.push_back((*column_set[j]).to_sexp());
    }

    Rcpp::List out = Rcpp::wrap(tmp);
    out.names() = col_names;

    return out;
  };
  void close(){
    res_ptr->close();
  };
  void close_and_release(){
    res_ptr->close();
    res_ptr.release();
  };
  ~result(){};
};

}

#endif
