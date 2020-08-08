#include "hyperapi/hyperapi.hpp"
#include <Rcpp.h>
#include <string>
#include "rhyper_types.h"

typedef std::shared_ptr<hyperapi::HyperProcess> HyperProcessPtr;
typedef std::shared_ptr<hyperapi::Connection> HyperConnectionPtr;

// [[Rcpp::export]]
SEXP create_hyper_process(SEXP telemetry){
  auto telemetry_ = Rcpp::as<bool>(telemetry);
  HyperProcessPtr* hp = new HyperProcessPtr(new hyperapi::HyperProcess(hyperapi::Telemetry::DoNotSendUsageDataToTableau));
  return Rcpp::XPtr<HyperProcessPtr>(hp, true);
}

// [[Rcpp::export]]
SEXP create_hyper_connection(SEXP process_ptr, Rcpp::List database){
  Rcpp::XPtr<HyperProcessPtr> hp(process_ptr);
  auto endpoint_ = hp->get()->getEndpoint();
  if(database.size() == 0){
    HyperConnectionPtr* hc = new HyperConnectionPtr(new hyperapi::Connection(endpoint_));
    return Rcpp::XPtr<HyperConnectionPtr>(hc, true);
  }else{
    auto db_ = Rcpp::as<std::string>(database[0]);
    HyperConnectionPtr* hc = new HyperConnectionPtr(new hyperapi::Connection(endpoint_, db_));
    return Rcpp::XPtr<HyperConnectionPtr>(hc, true);
  }
}

// [[Rcpp::export]]
void disconnect(SEXP connection_ptr){
  Rcpp::XPtr<HyperConnectionPtr> hc(connection_ptr);
  hc->get()->close();
}

// [[Rcpp::export]]
void terminate(SEXP process_ptr){
  Rcpp::XPtr<HyperProcessPtr> hp(process_ptr);
  hp->get()->close();
}

// [[Rcpp::export]]
void attach_database(SEXP connection_ptr, SEXP db_file){

  Rcpp::XPtr<HyperConnectionPtr> hc(connection_ptr);
  std::string db_file_ = Rcpp::as<std::string>(db_file);
  hc->get()->getCatalog().attachDatabase(db_file_);

}

// [[Rcpp::export]]
void detach_database(SEXP connection_ptr, SEXP db_name){

  Rcpp::XPtr<HyperConnectionPtr> hc(connection_ptr);
  std::string db_name_ = Rcpp::as<std::string>(db_name);
  hyperapi::DatabaseName dbn(db_name_);
  hc->get()->getCatalog().detachDatabase(dbn);

}

// [[Rcpp::export]]
void query_execute(SEXP conn, SEXP statement){

  Rcpp::XPtr<HyperConnectionPtr> hc(conn);
  std::string statement_ = Rcpp::as<std::string>(statement);

  hc->get()->executeCommand(statement_);

}

namespace Rcpp {

template<>
hyperapi::HyperProcess* as(SEXP x){
  HyperProcessPtr* hp = (HyperProcessPtr*)(R_ExternalPtrAddr(x));

  if(!hp){
    stop("Invalid Hyper Process.");
  }

  return hp->get();

}

template<>
hyperapi::Connection* as(SEXP x){
  HyperConnectionPtr* hc = (HyperConnectionPtr*)(R_ExternalPtrAddr(x));

  if(!hc){
    stop("Invalid connection.");
  }

  return hc->get();

}

}
