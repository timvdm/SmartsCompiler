
# the unit tests
set(tests
  optimizer
  scores
  )

set(TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# define TESTDATADIR for tests that need input files
add_definitions(-DTESTDATADIR="${CMAKE_BINARY_DIR}/")


foreach(test ${tests})
  add_executable(test_${test} ${test}.cpp)
  target_link_libraries(test_${test} smartscompiler)
  add_test(${test}_Test ${TEST_PATH}/test_${test})
  set_tests_properties(${test}_Test PROPERTIES
    FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Test failed")
endforeach(test ${tests})


