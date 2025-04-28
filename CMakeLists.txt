cmake_minimum_required(VERSION 4.0)

project(choc_stb)

set(CHOC_BUILD_TEST OFF CACHE BOOL "Build the test")

set(CMAKE_CXX_STANDARD 23)
if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-etc")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

set(CHOC_SOURCES
    StringUtils.hpp
    UTF8.hpp
    FileWatch.hpp
    TextTable.hpp
    glob.hpp
)


# Create an interface library for header-only usage
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE ".")
# target_compile_features(${PROJECT_NAME}  PUBLIC cxx_std_23)

if (CHOC_BUILD_TEST)
    add_subdirectory(tests)
endif()