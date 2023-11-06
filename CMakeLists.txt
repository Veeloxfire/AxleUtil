cmake_minimum_required(VERSION 3.10)
project(AxleUtil)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED TracerINCLUDE)
  message(FATAL_ERROR "Requires TracerINCLUDE to compile, even Tracer is not being used")
endif()

add_library(AxleUtil STATIC)
target_include_directories(AxleUtil PUBLIC "${PROJECT_SOURCE_DIR}/include" ${TracerINCLUDE})

if(AxleTRACING)
  target_compile_definitions(AxleUtil PRIVATE TRACING_ENABLE)

  if(NOT DEFINED TracerBIN)
    message(FATAL_ERROR "Requires TracerBIN to compile with tracing enabled")
  endif()

  target_link_libraries(AxleUtil "${TracerBIN}/Tracer.lib")
endif()

if(AxleTHROWING_ASSERTS)
  target_compile_definitions(AxleUtil PRIVATE ASSERT_EXCEPTIONS)
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
)

target_sources(AxleUtil PRIVATE ${CppFiles})
target_sources(AxleUtil PRIVATE 
  FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CppHeaders}
)