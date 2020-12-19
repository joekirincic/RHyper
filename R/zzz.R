
if(.Platform$OS.type == "windows"){
  loc_hyper_exec <- fs::path_join(c(fs::path_wd(), "lib", "bin", "tableauhyperapi.dll"))
  dyn.load(loc_hyper_exec)
}

.onLoad <- function(libname, pkgname){
  Rcpp::registerPlugin(name = "RHyper", plugin = RHyper:::inlineCxxPlugin())
  invisible()
}

