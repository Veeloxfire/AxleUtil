cmake_minimum_required(VERSION 3.10)
project(AxleUtilTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(AxleTRACING)
  message(FATAL_ERROR "Tests are not allowed with tracing")
endif()

add_executable(AxleUtilTests)
set_target_properties(AxleUtilTests PROPERTIES OUTPUT_NAME "AxleUtilTests$<CONFIG>")

if(AxleTHROWING_ASSERTS)
  target_compile_definitions(AxleUtilTests PUBLIC ASSERT_EXCEPTIONS)
endif()

if(AxleTestSANITY)
  target_compile_definitions(AxleUtil PUBLIC AXLE_TEST_SANITY)
endif()

target_link_libraries(AxleUtilTests AxleUtil)

set(CppFiles
  "${PROJECT_SOURCE_DIR}/containers_tests.cpp"
  "${PROJECT_SOURCE_DIR}/format_tests.cpp"
  "${PROJECT_SOURCE_DIR}/files_tests.cpp"
  "${PROJECT_SOURCE_DIR}/math_tests.cpp"
  "${PROJECT_SOURCE_DIR}/string_tests.cpp"

  "${PROJECT_SOURCE_DIR}/test_main.cpp"
)

target_sources(AxleUtilTests PRIVATE ${CppFiles})
