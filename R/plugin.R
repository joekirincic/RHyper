

inlineCxxPlugin <- function(...){
  incl <- c(
    "#include <string>",
    "#include <tuple>",
    "#include <list>",
    "#include <forward_list>",
    "#include <memory>",
    glue::glue("#include {glue::double_quote('hyperapi/hyperapi.hpp')}")
  )

  Rcpp::Rcpp.plugin.maker(
  include.before = incl,
  ## FIND dll/so file at pkg root file.path(find.package('hyprflex'), "libs", .Platform$r_arch, glue("hyprflex{.Platform$dynlib.ext}")),
  libs = "/Users/Joe/Desktop/r-projects/hyprflex/lib/libtableauhyperapi.dylib",
  package = "hyprflex"
  )
}
