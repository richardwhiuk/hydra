# Hydra Project Build File

cmake_minimum_required (VERSION 2.6)

project(hydra)

set (HYDRA_VERSION_MAJOR 0)
set (HYDRA_VERSION_MINOR 9)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall -Werror")

configure_file (
  "${PROJECT_SOURCE_DIR}/include/hydra.hpp.in"
  "${PROJECT_BINARY_DIR}/include/hydra.hpp"
  )

include_directories("include" "${PROJECT_BINARY_DIR}/include")

set(HEADER_FILES
  include/hydra.hpp
)

set(SOURCE_FILES
  src/main.cpp 
)

add_executable(hydra ${HEADER_FILES} ${SOURCE_FILES} ${CGI_HEADER_FILES} ${CGI_SOURCE_FILES})

INSTALL(TARGETS hydra DESTINATION ${PROJECT_SOURCE_DIR}/bin)
