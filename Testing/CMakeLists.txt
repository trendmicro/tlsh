# set(CTEST_OUTPUT_ON_FAILURE true)
# cmake bug - the above does not work, call "make test --output-on-failure"
# instead
add_test(tlsh_unittest_len "${CMAKE_SOURCE_DIR}/Testing/test.sh")
add_test(tlsh_unittest_xlen "${CMAKE_SOURCE_DIR}/Testing/test.sh" "-xlen")
