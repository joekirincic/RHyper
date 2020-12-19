# Prepare your package for installation here.
# Use 'define()' to define configuration variables.
# Use 'configure_file()' to substitute configuration values.

os <- Sys.info()["sysname"]

if(!(os %in% c("Linux", "Windows"))){
  # OS is a Mac.
  hapi_lib_loc <- glue::glue('{glue::double_quote(file.path(path.expand("~"), "Library", "Tableau Hyper API", "cpp"))}')
}

# Define the location of the Hyper API.
define(HAPI_LIB_LOC = hapi_lib_loc)

# Require C++17
define(CPP_SPEC = "CXX17")

# Define the location of the Hyper API header files.
hapi_includes_loc <- file.path("..", "inst", "include")
define(HAPI_INCLUDES = hapi_includes_loc)

configure_file("src/Makevars.in")
