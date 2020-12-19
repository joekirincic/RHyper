#include "hyperapi/hyperapi.hpp"
#include <filesystem>
#include "connection.h"
#include <Rcpp.h>

namespace fs = std::filesystem;

typedef std::shared_ptr<RHyper::result> result_ptr;
typedef std::unique_ptr<RHyper::connection> conn_ptr;

std::string file_name(std::string path_){

  const size_t last_slash_idx = path_.find_last_of("\\/");
  if (std::string::npos != last_slash_idx)
  {
    path_.erase(0, last_slash_idx + 1);
  }

  const size_t period_idx = path_.rfind('.');
  if (std::string::npos != period_idx)
  {
    path_.erase(period_idx);
  }

  return path_;
}

namespace RHyper {
void connection::disconnect(){
  conn_ptr->close();
  proc_ptr->close();
};

bool connection::is_open(){
  return conn_ptr->isOpen();
};

bool connection::is_busy(){
  return !conn_ptr->isReady();
};

void connection::attach_database(const std::string& db_name_, const std::string& db_alias_){
  std::string sql_cmd = "ATTACH DATABASE " + hyperapi::escapeName(db_name_) + " AS " + hyperapi::escapeName(db_alias_);
  conn_ptr->executeCommand(sql_cmd);
  db_name.push_back(db_name_);
};

void connection::detach_database(const std::string& db_name_){
  fs::path db(db_name_);
  std::string sql_cmd = "DETACH DATABASE " + db.filename().string();
  conn_ptr->executeCommand(sql_cmd);
  db_name.erase(std::remove(db_name.begin(), db_name.end(), db.string()), db_name.end());
};

result_ptr connection::execute_query(std::string sql){

  if(auto current_res = res_ptr.lock()){
    // If we enter this block, then there is an
    // active result set we need to close.
    Rcpp::warning("Releasing active result set.");
    current_res->close_and_release();
  }
  std::unique_ptr<hyperapi::Result> r = std::unique_ptr<hyperapi::Result>(new hyperapi::Result());
  *r = conn_ptr->executeQuery(sql);
  std::unique_ptr<hyperapi::ResultIterator> s = std::unique_ptr<hyperapi::ResultIterator>(new hyperapi::ResultIterator(*r, hyperapi::IteratorBeginTag()));
  std::unique_ptr<hyperapi::ResultIterator> e = std::unique_ptr<hyperapi::ResultIterator>(new hyperapi::ResultIterator(*r, hyperapi::IteratorEndTag()));
  auto out = std::shared_ptr<RHyper::result>(new RHyper::result(r, s, e, sql));
  set_current_result(out);
  return out;
};

int64_t connection::execute_command(std::string sql){

  auto out = conn_ptr->executeCommand(sql);

  return out;

};

void connection::set_current_result(std::shared_ptr<result> r){
  res_ptr = r;
};

void connection::close_current_result(){
  auto r = res_ptr.lock();
  Rcpp::warning("Closing current result set...");
  r->close();
  r.reset();
};

bool can_create_connection(){
  try{
    std::unique_ptr<hyperapi::HyperProcess> hp = std::unique_ptr<hyperapi::HyperProcess>(new hyperapi::HyperProcess());
    std::unique_ptr<hyperapi::Connection> hc = std::unique_ptr<hyperapi::Connection>(new hyperapi::Connection(hp->getEndpoint()));
    connection conn = connection(hp, hc);
    return true;
  }
  catch(...){
    return false;
  }

};

};

typedef std::unique_ptr<RHyper::connection> conn_ptr;

// [[Rcpp::export]]
SEXP connect(
    Rcpp::Nullable<Rcpp::CharacterVector> database_ = R_NilValue,
    Rcpp::Nullable<Rcpp::CharacterVector> aliases_ = R_NilValue
){
  return connect_impl(database_, aliases_);
}


SEXP connect_impl(
    Rcpp::Nullable<Rcpp::CharacterVector> database_,
    Rcpp::Nullable<Rcpp::CharacterVector> aliases_
){

  std::unique_ptr<hyperapi::HyperProcess> hp(new hyperapi::HyperProcess());
  *hp = hyperapi::HyperProcess(hyperapi::Telemetry::DoNotSendUsageDataToTableau);
  std::unique_ptr<hyperapi::Connection> hc(new hyperapi::Connection());
  *hc = hyperapi::Connection(hp->getEndpoint());
  conn_ptr* out = new conn_ptr(new RHyper::connection(hp, hc));

  if(database_.isNotNull()){
    auto database = Rcpp::as<std::vector<std::string>>(database_.get());
    auto alias = Rcpp::as<std::vector<std::string>>(aliases_.get());
    for(int i = 0; i < database.size(); i++){
      auto d = database[i];
      auto a = alias[i];
      out->get()->attach_database(d, a);
    }
    // for(auto db: database){
    //   out->get()->attach_database(db);
    // }
  }

  return Rcpp::XPtr<conn_ptr>(out, true);
}

// [[Rcpp::export]]
void disconnect(SEXP connection_ptr){
  auto hc = Rcpp::XPtr<conn_ptr>(connection_ptr).get();
  if(hc->get()->is_open()){
    if(hc->get()->is_busy()){
      Rcpp::warning("Connection is closed but a result set is still open.");
    }
    hc->get()->disconnect();
    return;
  }
  Rcpp::warning("The connection is already closed.");
}

// [[Rcpp::export]]
void execute_command(SEXP conn_, SEXP statement_){

  auto conn = Rcpp::XPtr<conn_ptr>(conn_).get();
  std::string statement = Rcpp::as<std::string>(statement_);

  conn->get()->execute_command(statement);

  return;
}

// [[Rcpp::export]]
bool is_valid_connection(SEXP conn_){
  auto conn = Rcpp::XPtr<conn_ptr>(conn_).get();
  return conn;
}

// // [[Rcpp::export]]
// SEXP db_detach_impl(SEXP conn_, Rcpp::Nullable<Rcpp::CharacterVector> file_){
//   auto conn = Rcpp::XPtr<conn_ptr>(conn_).get();
//   if(file_.isNotNull()){
//     auto fls = Rcpp::as<std::vector<std::string>>(file_.get());
//     auto files = Rcpp::as<std::vector<fs::path>>(fs::path(Rcpp::as<std::string>(file_.get())));
//     for(auto f: files){
//       out->get()->attach_database(db);
//     }
//   }
// };

// [[Rcpp::export]]
SEXP file_name_impl(SEXP path_){

  std::string path = Rcpp::as<std::string>(path_);
  std::string out = file_name(path);

  return Rcpp::wrap(out);
}
