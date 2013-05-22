# set(CTEST_OUTPUT_ON_FAILURE true)
# cmake bug - the above does not work, call "make test --output-on-failure"
# instead
if(TLSH_SHORT EQUAL 1)
  add_test(tlsh_unittest_compact "${CMAKE_SOURCE_DIR}/Testing/test.sh")
else()
  add_test(tlsh_unittest_full "${CMAKE_SOURCE_DIR}/Testing/test_long.sh")
  add_test(tlsh_unittest_full_xlen "${CMAKE_SOURCE_DIR}/Testing/test_long_xlen.sh")
endif()
