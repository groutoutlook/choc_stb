cmake_minimum_required(VERSION 4.0)

project(choc_stb)

set(CHOC_BUILD_TEST OFF CACHE BOOL "Build the test")

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-etc")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

set(CHOC_SOURCES
    StringUtils.hpp
    UTF8.hpp
    FileWatch.hpp
    TextTable.hpp
)

add_library(${PROJECT_NAME}  STATIC ${CHOC_SOURCES} )
target_include_directories(${PROJECT_NAME}  PUBLIC ".")
target_compile_features(${PROJECT_NAME}  PUBLIC cxx_std_23)

if (CHOC_BUILD_TEST)
    add_subdirectory(tests)
endif()