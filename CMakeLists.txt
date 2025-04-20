cmake_minimum_required(VERSION 3.10)

project(AxleUtilProj
  VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND DEFINED ENV{CMAKE_INSTALL_PREFIX})
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE $ENV{CMAKE_INSTALL_PREFIX})
endif()

message("Install Prefix: ${CMAKE_INSTALL_PREFIX}")

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

add_library(Core STATIC)
set_target_properties(Core PROPERTIES OUTPUT_NAME "AxleUtil$<CONFIG>")

add_library(TestServer STATIC)
set_target_properties(TestServer PROPERTIES OUTPUT_NAME "AxleTestServer$<CONFIG>")

add_library(TestClient STATIC)
set_target_properties(TestClient PROPERTIES OUTPUT_NAME "AxleTestClient$<CONFIG>")

target_link_libraries(TestServer Core)
target_link_libraries(TestClient Core)
add_dependencies(TestServer TestClient) # Client must be up to date

target_include_directories(Core PUBLIC 
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:AxleUtil/include/>)

if(MSVC)
  target_compile_options(Core PUBLIC /diagnostics:caret /Wall /WX /wd4514 /wd4582 /wd4583 /wd4623 /wd4625 /wd4626 /wd4627 /wd4710 /wd4711 /wd4820 /wd5027 /wd5039 /wd5045 /wd5220 /permissive-)
else()
  target_compile_options(Core PUBLIC -Wall -Wextra -Wpedantic -Werror)
endif()

target_compile_definitions(Core PUBLIC STACKTRACE_ENABLE)

option(AxleTRACING "Enable execution tracing" OFF)
if(AxleTRACING)
  target_compile_definitions(Core PUBLIC AXLE_UTIL_TRACING_ENABLE)
  if(NOT DEFINED TracerINCLUDE)
    message(FATAL_ERROR "Requires TracerINCLUDE to compile with tracing enabled")
  endif()
  
  target_include_directories(Core PUBLIC ${TracerINCLUDE})

  if(NOT DEFINED TracerBIN)
    message(FATAL_ERROR "Requires TracerBIN to compile with tracing enabled")
  endif()

  target_link_libraries(Core "${TracerBIN}/Tracer.lib")
endif()

option(AxleTestSANITY "Enable Sanity Tests" OFF)
if(AxleTestSANITY)
  message("Enabled: sanity checks")
  target_compile_definitions(TestClient PUBLIC AXLE_TEST_SANITY)
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
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/files_base.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/format.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/formattable.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/hash.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/io.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/math.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/memory.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/option.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/panic.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/primitives.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/safe_lib.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/serialize.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stacktrace.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/strings.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/threading.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/tracing_wrapper.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/utility.h"

  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/compare.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/string.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/stdext/vector.h"
  
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/os/os_windows.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/os/os_windows_files.h"

  "${PROJECT_SOURCE_DIR}/include/AxleTest/unit_tests.h"
  "${PROJECT_SOURCE_DIR}/include/AxleTest/ipc.h"
)

target_sources(Core PRIVATE ${UtilSourceFiles})
target_sources(Core PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

target_sources(TestServer PRIVATE ${TestServerFiles})
target_sources(TestServer PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

target_sources(TestClient PRIVATE ${TestClientFiles})
target_sources(TestClient PRIVATE
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

#### Unit tests ####

add_executable(UnitTestServer)
add_executable(UnitTestClient)

set_target_properties(UnitTestServer PROPERTIES OUTPUT_NAME "TestServer$<CONFIG>")
set_target_properties(UnitTestClient PROPERTIES OUTPUT_NAME "TestClient$<CONFIG>")

target_link_libraries(UnitTestServer TestServer)
target_link_libraries(UnitTestClient TestClient)
add_dependencies(UnitTestServer UnitTestClient)

target_compile_definitions(UnitTestServer PRIVATE AXLE_TEST_CLIENT_EXE="TestClient$<CONFIG>.exe")

foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${config_type} config_upper)
  set_target_properties(UnitTestServer UnitTestClient
      PROPERTIES
      "ARCHIVE_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
      "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
      "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${PROJECT_BINARY_DIR}/out/test"
  )
endforeach()

set(TestFiles
  "${PROJECT_SOURCE_DIR}/tests/args_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/containers_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/files_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/format_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/hash_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/math_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/memory_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/option_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/serialize_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stacktrace_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/string_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/testing_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/thread_tests.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stdext/string.cpp"
  "${PROJECT_SOURCE_DIR}/tests/stdext/vector.cpp"
)

target_sources(UnitTestServer PRIVATE "${PROJECT_SOURCE_DIR}/tests/test_server_main.cpp")
target_sources(UnitTestServer PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES "${PROJECT_SOURCE_DIR}/tests/test_contexts.h"
)

target_sources(UnitTestClient PRIVATE "${PROJECT_SOURCE_DIR}/tests/test_client_main.cpp" ${TestFiles})
target_sources(UnitTestClient PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES "${PROJECT_SOURCE_DIR}/tests/test_contexts.h"
)

#### install ####

install(
  TARGETS Core
  EXPORT AxleUtilTargets
  DESTINATION AxleUtil/lib/
)

install(
  TARGETS TestServer
  EXPORT AxleUtilTargets
  DESTINATION AxleUtil/lib/
)

install(
  TARGETS TestClient
  EXPORT AxleUtilTargets
  DESTINATION AxleUtil/lib/
)

install(
  EXPORT AxleUtilTargets
  NAMESPACE AxleUtil::
  DESTINATION AxleUtil/
)

install(DIRECTORY include DESTINATION AxleUtil)

include(CMakePackageConfigHelpers)

configure_package_config_file( 
  "Config.cmake.in" 
  "AxleUtilConfig.cmake"
  INSTALL_DESTINATION AxleUtil/
)

write_basic_package_version_file(
  "AxleUtilConfigVersion.cmake"
  COMPATIBILITY ExactVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AxleUtilConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AxleUtilConfigVersion.cmake"
  DESTINATION AxleUtil
)
