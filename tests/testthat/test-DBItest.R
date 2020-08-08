DBItest::make_context(Hyper(), NULL, tweaks = DBItest::tweaks(omit_blob_tests = TRUE))
DBItest::test_getting_started()
DBItest::test_driver(
  skip = c(
    "connect_bigint.*",
    NULL
  )
)
DBItest::test_connection()
