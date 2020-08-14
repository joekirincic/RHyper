# RHyper <img src='man/figures/logo.png' align="right" height="139" />
A DBI-based interface to Tableau's Hyper Data Engine.

## Installation
```R
# Install the latest version using devtools:
# install.packages("devtools")
devtools::install_github("joekirincic/RHyper",)

# Or using remotes:
# install.packages("remotes")
remotes::install_github("joekirincic/RHyper")
```

## Basic usage

```R
library(DBI)
# Connect to a .hyper file.
hyper_file <- "mydb.hyper"
con <- dbConnect(RHyper::Hyper(), hyper_file)

dbListTables(con)
dbWriteTable(con, "mtcars", mtcars)
dbListTables(con)

dbListFields(con, "mtcars")
dbReadTable(con, "mtcars")

# You can fetch all results:
res <- dbSendQuery(con, "SELECT * FROM mtcars WHERE cyl = 4")
dbFetch(res)
dbClearResult(res)

# Or a chunk at a time
res <- dbSendQuery(con, "SELECT * FROM mtcars WHERE cyl = 4")
while(!dbHasCompleted(res)){
  chunk <- dbFetch(res, n = 5)
  print(nrow(chunk))
}
# Clear the result
dbClearResult(res)

# Disconnect from the database
dbDisconnect(con)
```
