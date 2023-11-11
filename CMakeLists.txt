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

target_include_directories(AxleUtil PUBLIC "${PROJECT_SOURCE_DIR}/include")

option(AxleTRACING "Enable execution tracing" OFF)
if(AxleTRACING)
  target_compile_definitions(AxleUtil PRIVATE TRACING_ENABLE AXLE_TRACING)
  if(NOT DEFINED TracerINCLUDE)
    message(FATAL_ERROR "Requires TracerINCLUDE to compile with tracing enabled")
  endif()
  
  target_include_directories(AxleUtil PUBLIC ${TracerINCLUDE})

  if(NOT DEFINED TracerBIN)
    message(FATAL_ERROR "Requires TracerBIN to compile with tracing enabled")
  endif()

  target_link_libraries(AxleUtil "${TracerBIN}/Tracer.lib")
endif()

option(AxleTHROWING_ASSERTS "Enable Throwing Asserts" OFF)
if(AxleTHROWING_ASSERTS)
  target_compile_definitions(AxleUtil PUBLIC ASSERT_EXCEPTIONS)
endif()

set(CppFiles
  "${PROJECT_SOURCE_DIR}/src/files.cpp"
  "${PROJECT_SOURCE_DIR}/src/format.cpp"
  "${PROJECT_SOURCE_DIR}/src/io.cpp"
  "${PROJECT_SOURCE_DIR}/src/memory.cpp"
  "${PROJECT_SOURCE_DIR}/src/strings.cpp"
  "${PROJECT_SOURCE_DIR}/src/threading.cpp"
  "${PROJECT_SOURCE_DIR}/src/utility.cpp"

  "${PROJECT_SOURCE_DIR}/src/os/os_windows.cpp"

  "${PROJECT_SOURCE_DIR}/src/unit_tests.cpp"
)

set(CppHeaders
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/args.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/files.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/format.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/formattable.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/io.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/memory.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/safe_lib.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/strings.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/threading.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/utility.h"
  "${PROJECT_SOURCE_DIR}/include/AxleUtil/os/os_windows.h"

  "${PROJECT_SOURCE_DIR}/include/AxleTest/unit_tests.h"
)

target_sources(AxleUtil PRIVATE ${CppFiles})
target_sources(AxleUtil PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)

option(BUILD_TESTS "Build the tests" OFF)

if(BUILD_TESTS)
  add_subdirectory("./tests")
endif()
