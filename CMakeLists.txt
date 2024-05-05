cmake_minimum_required(VERSION 3.10)
project(AxleUtil)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add Static Configs
set(new_config_types "")
foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${config_type} config_upper)
  list(APPEND new_config_types "${config_type}Static")
  set("CMAKE_CXX_FLAGS_${config_upper}STATIC" "${CMAKE_CXX_FLAGS_${config_upper}}")


  if(MSVC)
    if(config_type STREQUAL Debug)
      set("CMAKE_CXX_FLAGS_${config_upper}STATIC" "${CMAKE_CXX_FLAGS_${config_upper}STATIC} /MTd")
    else()
      set("CMAKE_CXX_FLAGS_${config_upper}STATIC" "${CMAKE_CXX_FLAGS_${config_upper}STATIC} /MT")
    endif()
  endif()

  set("CMAKE_EXE_LINKER_FLAGS_${config_upper}STATIC" "${CMAKE_EXE_LINKER_FLAGS_${config_upper}}")
endforeach()

list(APPEND CMAKE_CONFIGURATION_TYPES ${new_config_types})

foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${config_type} config_upper)
  set("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out")
  set("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out")
  set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out")
endforeach()

add_library(AxleUtil STATIC)
set_target_properties(AxleUtil PROPERTIES OUTPUT_NAME "AxleUtil$<CONFIG>")

add_library(AxleTestServer STATIC)
set_target_properties(AxleTestServer PROPERTIES OUTPUT_NAME "AxleTestServer$<CONFIG>")

add_library(AxleTestClient STATIC)
set_target_properties(AxleTestClient PROPERTIES OUTPUT_NAME "AxleTestClient$<CONFIG>")

target_link_libraries(AxleTestServer AxleUtil)
target_link_libraries(AxleTestClient AxleUtil)
add_dependencies(AxleTestServer AxleTestClient) # Client must be up to date

target_compile_definitions(AxleTestServer PRIVATE AXLE_TEST_CLIENT_EXE="AxleTestClient$<CONFIG>.exe")

target_include_directories(AxleUtil PUBLIC "${PROJECT_SOURCE_DIR}/include")

if(MSVC)
  target_compile_options(AxleUtil PUBLIC /W4 /WX /permissive-)
else()
  target_compile_options(AxleUtil PUBLIC -Wall -Wextra -Wpedantic -Werror)
endif()

target_compile_definitions(AxleUtil PUBLIC STACKTRACE_ENABLE)

option(AxleTRACING "Enable execution tracing" OFF)
if(AxleTRACING)
  target_compile_definitions(AxleUtil PUBLIC TRACING_ENABLE AXLE_TRACING)
  if(NOT DEFINED TracerINCLUDE)
    message(FATAL_ERROR "Requires TracerINCLUDE to compile with tracing enabled")
  endif()
  
  target_include_directories(AxleUtil PUBLIC ${TracerINCLUDE})

  if(NOT DEFINED TracerBIN)
    message(FATAL_ERROR "Requires TracerBIN to compile with tracing enabled")
  endif()

  target_link_libraries(AxleUtil "${TracerBIN}/Tracer.lib")
endif()

option(AxleTestSANITY "Enable Sanity Tests" OFF)
if(AxleTestSANITY)
  message("Enabled: sanity checks")
  target_compile_definitions(AxleTestClient PUBLIC AXLE_TEST_SANITY)
endif()

set(UtilSourceFiles
  "${PROJECT_SOURCE_DIR}/src/files.cpp"
  "${PROJECT_SOURCE_DIR}/src/format.cpp"
  "${PROJECT_SOURCE_DIR}/src/io.cpp"
  "${PROJECT_SOURCE_DIR}/src/memory.cpp"
  "${PROJECT_SOURCE_DIR}/src/strings.cpp"
  "${PROJECT_SOURCE_DIR}/src/threading.cpp"
  "${PROJECT_SOURCE_DIR}/src/utility.cpp"

  "${PROJECT_SOURCE_DIR}/src/os/os_windows.cpp"
  "${PROJECT_SOURCE_DIR}/src/os/os_windows_files.cpp"
)

set(TestClientFiles
  "${PROJECT_SOURCE_DIR}/AxleTest/client/client.cpp"
)

set(TestServerFiles
  "${PROJECT_SOURCE_DIR}/AxleTest/server/server.cpp"
)

set(CppHeaders
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/args.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/files.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/format.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/formattable.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/io.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/memory.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/safe_lib.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/serialize.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stacktrace.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/strings.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/threading.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/utility.h"

  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/compare.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/string.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/vector.h"
  
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/os/os_windows.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/os/os_windows_files.h"

  "${PROJECT_SOURCE_DIR}/include/AxleTest/unit_tests.h"
  "${PROJECT_SOURCE_DIR}/include/AxleTest/ipc.h"
)

target_sources(AxleUtil PRIVATE ${UtilSourceFiles})
target_sources(AxleUtil PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

target_sources(AxleTestServer PRIVATE ${TestServerFiles})
target_sources(AxleTestServer PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

target_sources(AxleTestClient PRIVATE ${TestClientFiles})
target_sources(AxleTestClient PRIVATE
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

#### Unit tests

add_executable(TestServer)
add_executable(TestClient)

set_target_properties(TestServer PROPERTIES OUTPUT_NAME "TestServer$<CONFIG>")
set_target_properties(TestClient PROPERTIES OUTPUT_NAME "TestClient$<CONFIG>")

target_link_libraries(TestServer AxleTestServer)
target_link_libraries(TestClient AxleTestClient)
add_dependencies(TestServer TestClient)

target_compile_definitions(TestServer PRIVATE AXLE_TEST_CLIENT_EXE="TestClient$<CONFIG>.exe")

foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${config_type} config_upper)
  set_target_properties(TestServer TestClient
      PROPERTIES
      "ARCHIVE_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
      "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
      "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
  )
endforeach()

set(TestFiles
  "${PROJECT_SOURCE_DIR}/tests/args_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/containers_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/format_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/serialize_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stacktrace_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/files_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/math_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/string_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/testing_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/thread_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stdext/string.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stdext/vector.cpp"
)

target_sources(TestServer PRIVATE "${PROJECT_SOURCE_DIR}/tests/test_server_main.cpp")
target_sources(TestClient PRIVATE "${PROJECT_SOURCE_DIR}/tests/test_client_main.cpp" ${TestFiles})

