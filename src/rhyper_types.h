
#ifndef __RHYPER_TYPES__
#define __RHYPER_TYPES__

#include "hyperapi/hyperapi.hpp"
#include <Rcpp.h>
#include <memory>

typedef std::shared_ptr<hyperapi::HyperProcess> HyperProcessPtr;
typedef std::shared_ptr<hyperapi::Connection> HyperConnectionPtr;
typedef std::shared_ptr<hyperapi::ResultIterator> ResultIteratorPtr;
typedef std::unique_ptr<hyperapi::Result> ResultPtr;

namespace Rcpp {

template<>
HyperProcessPtr* as(SEXP x);

template<>
HyperConnectionPtr* as(SEXP x);

template<>
ResultIteratorPtr* as(SEXP x);

template<>
ResultPtr* as(SEXP x);

}

#endif
