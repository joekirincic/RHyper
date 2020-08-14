
if(.Platform$OS.type == "windows"){
  loc_hyper_exec <- fs::path_join(c(fs::path_wd(), "lib", "bin", "tableauhyperapi.dll"))
  dyn.load(loc_hyper_exec)
}

.RHyperSession <- NULL

.onLoad <- function(libname, pkgname){
  .RHyperSession <<- new.env()
  .RHyperSession$Process <- NULL
  .RHyperSession$Connection <- NULL
  .RHyperSession$Result$is_open <- NULL
  .RHyperSession$Result$iterator <- NULL
  Rcpp::registerPlugin(name = "RHyper", plugin = RHyper:::inlineCxxPlugin())
  invisible()
}

