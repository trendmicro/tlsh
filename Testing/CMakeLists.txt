# set(CTEST_OUTPUT_ON_FAILURE true)
# cmake bug - the above does not work, call "make test --output-on-failure"
# instead
add_test(tlsh_unittest_len_xlen "${CMAKE_SOURCE_DIR}/Testing/test.sh")
if(CMAKE_HOST_UNIX)
    if(CMAKE_SHARED_LIBRARY EQUAL 1)
        set_tests_properties(tlsh_unittest_len tlsh_unittest_xlen PROPERTIES ENVIRONMENT "LD_PRELOAD=${CMAKE_SOURCE_DIR}/lib/libtlsh.so.0")
    endif()
endif()
