#include <Rcpp.h>
#include <vector>
#include <list>
using namespace Rcpp;

// This is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp
// function (or via the Source button on the editor toolbar). Learn
// more about Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//   http://gallery.rcpp.org/
//

// [[Rcpp::export]]
SEXP f() {

  union S {
    int integer_;
    double double_;
    std::string string_;
    bool bool_;
    ~S() {};
  };

  S vessel = {.bool_ = false};
  S vessel2 = {.bool_ = false};

  std::list<double> v;
  std::list<bool> v2;

  for(int i = 0; i < 1000000; i++){
    if(i % 2 == 0){
      vessel.double_ = 3.14;
      // vessel2.bool_ = true;
      v.push_back(vessel.double_);
      vessel.bool_ = true;
      v2.push_back(vessel.bool_);
    }else{
      vessel.double_ = 69.69;
      v.push_back(vessel.double_);
      vessel.bool_ = false;
      v2.push_back(vessel.bool_);
    }
  }

  auto df = DataFrame::create(_["stringsAsFactors"] = false);
  // DoubleVector out = wrap(v);
  df.push_back(v, "my_double");
  df.push_back(v2, "my_logical");
  std::vector<int> x(3);

  return df;
}

enum TAG {
  INT_,
  DOUBLE_,
  BOOL_
};

union H {
  int integer_;
  double double_;
  bool bool_;
};

union V {
  int integer_;
  double double_;
  bool bool_;
};

inline V get_int(H x){
  return {.integer_ = x.integer_};
}

inline V get_double(H x){
  return {.double_ = x.double_};
}

inline V get_bool(H x){
  return {.bool_ = x.bool_};
}

RObject get_int_vector(std::list<V> v, bool is_nullable){
  R_xlen_t n = v.size();
  IntegerVector out(n);
  if(is_nullable){
    // out = std::transform(v.begin(), v.end(), out.begin(), [](V vi){ return vi.integer_; });
    int i = 0;
    for(auto it = v.begin(); it != v.end(); ++it){
      out[i] = it->integer_;
      i++;
    }
    return out;
  }
  int i = 0;
  for(auto it = v.begin(); it != v.end(); ++it){
    out[i] = it->integer_;
    i++;
  }
  return out;
};


RObject get_double_vector(std::list<V> v, bool is_nullable){
  R_xlen_t n = v.size();
  DoubleVector out(n);
  if(is_nullable){
    int i = 0;
    for(auto it = v.begin(); it != v.end(); ++it){
      out[i] = it->double_;
      i++;
    }
    return out;
  }
  int i = 0;
  for(auto it = v.begin(); it != v.end(); ++it){
    out[i] = it->double_;
    i++;
  }
  return out;
};

RObject get_bool_vector(std::list<V> v, bool is_nullable){
  R_xlen_t n = v.size();
  LogicalVector out(n);
  if(is_nullable){
    int i = 0;
    for(auto it = v.begin(); it != v.end(); ++it){
      out[i] = it->bool_;
      i++;
    }
    return out;
  }
  int i = 0;
  for(auto it = v.begin(); it != v.end(); ++it){
    out[i] = it->bool_;
    i++;
  }
  return out;
};

class Col {
  std::function<V(H)> read_fn;
  std::function<RObject(std::list<V>, bool)> transform_fn;
  std::list<V> vals = {};
public:
  enum TAG {
    INT_,
    DOUBLE_,
    BOOL_
  };
  Col(Col::TAG t){
    switch(t){
    case TAG::BOOL_:
    {
      read_fn = get_bool;
      transform_fn = get_bool_vector;
      break;
    }
    case TAG::INT_:
    {
      read_fn = get_int;
      transform_fn = get_int_vector;
      break;
    }
    default:
    {
      read_fn = get_double;
      transform_fn = get_double_vector;
    }
    }
  };
  void read_val(H h){
    V val = this->read_fn(h);
    this->vals.push_back(val);
  }
  bool nullable(){
    // This function would run something like
    // std::any_of(vals.begin(), vals.end(), is_null)
    return false;
  };
  RObject getColumn(){
    return this->transform_fn(this->vals, this->nullable());
  }
};

// [[Rcpp::export]]
RObject g(){

  std::list<V> data;
  data.push_back({.integer_ = 10});
  data.push_back({.integer_ = 20});
  data.push_back({.integer_ = 30});

  RObject out = get_int_vector(data, false);

  return out;

}

// [[Rcpp::export]]
SEXP h(){

  std::list<H> col1;
  std::list<H> col2;
  std::list<H> col3;

  for(int i = 0; i < 10000000; i++){
    col1.push_back({.integer_ = i});
    col2.push_back({.double_ = (i + 0.0)});
    col3.push_back({.bool_ = true});
  }

  std::vector<Col> data = {Col(Col::INT_), Col(Col::DOUBLE_), Col(Col::BOOL_)};

  for(auto it = col1.begin(), it2 = col2.begin(), it3 = col3.begin(); it != col1.end(); ++it, ++it2, ++it3){
    data[0].read_val(*it);
    data[1].read_val(*it2);
    data[2].read_val(*it3);
  }

  List out;
  out.attr("class") = "data.frame";

  for(size_t i = 0; i < data.size(); i++){
    out.push_back(data[i].getColumn());
  }

  out.names() = CharacterVector({"my_int", "my_dbl", "my_bool"});

  return out;
}

// [[Rcpp::export]]
SEXP z(){

  List out;
  out.attr("class") = CharacterVector({"tbl_df", "tbl", "data.frame"});

  return out;
}


// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically
// run after the compilation.
//

/*** R
x <- h()
*/
