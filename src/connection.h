
#ifndef __RHYPER_CON__
#define __RHYPER_CON__

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include "hyperapi/hyperapi.hpp"
#include "result.h"

typedef std::shared_ptr<RHyper::result> result_ptr;

namespace RHyper {

class connection {
private:
  std::unique_ptr<hyperapi::HyperProcess> proc_ptr;
  std::unique_ptr<hyperapi::Connection> conn_ptr;
  std::weak_ptr<result> res_ptr;
  std::vector<std::string> db_name;
public:
  connection(connection const &)=delete;
  connection &operator=(connection const &)=delete;
  connection(std::unique_ptr<hyperapi::HyperProcess> &p, std::unique_ptr<hyperapi::Connection> &c):
    proc_ptr(std::move(p)), conn_ptr(std::move(c)) {};
  connection(connection &&o):
    proc_ptr(std::move(o.proc_ptr)), conn_ptr(std::move(o.conn_ptr)), res_ptr(std::move(o.res_ptr)), db_name(std::move(o.db_name)) {};
  connection &operator=(connection &&o){
    if (this != &o)
    {
      proc_ptr = std::move(o.proc_ptr);
      conn_ptr = std::move(o.conn_ptr);
      res_ptr = std::move(o.res_ptr);
      db_name = std::move(o.db_name);
    }
    return *this;
  };
  void disconnect();
  bool is_open();
  bool is_busy();
  void attach_database(const std::string& db_name_, const std::string& db_alias_);
  void detach_database(const std::string& db_name_);
  void set_current_result(std::shared_ptr<result> r);
  void close_current_result();
  int64_t execute_command(std::string sql);
  result_ptr execute_query(std::string sql);
};

}

typedef std::unique_ptr<RHyper::connection> conn_ptr;

SEXP connect_impl(
    Rcpp::Nullable<Rcpp::CharacterVector> database_ = R_NilValue,
    Rcpp::Nullable<Rcpp::CharacterVector> aliases_ = R_NilValue
);

SEXP db_detach_impl(Rcpp::Nullable<Rcpp::CharacterVector> = R_NilValue);

#endif
