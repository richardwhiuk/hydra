#
# CMakeLists.txt
# ~~~~~~~~~~~~~~
#
# Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Hydra Project Build File
#

cmake_minimum_required (VERSION 2.6)

project(hydra)

set (HYDRA_VERSION_MAJOR 0)
set (HYDRA_VERSION_MINOR 9)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall -Werror")

set (HYDRA_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

find_package(Boost COMPONENTS thread system REQUIRED)

configure_file (
  "${PROJECT_SOURCE_DIR}/include/version.hpp.in"
  "${PROJECT_BINARY_DIR}/include/version.hpp"
  )

include_directories("include" "${PROJECT_BINARY_DIR}/include")

set(HEADER_FILES
	include/apache2.hpp
	include/config.hpp
	include/connection.hpp
	include/engine.hpp
	include/header.hpp
	include/host.hpp
	include/io_service_pool.hpp
	include/reply.hpp
	include/request.hpp
	include/request_handler.hpp
	include/request_parser.hpp
	include/server.hpp
	include/version.hpp
)

set(SOURCE_FILES
	src/apache2.cpp
	src/apache2_client.cpp
	src/config.cpp
	src/connection.cpp
	src/engine.cpp
	src/host.cpp
	src/io_service_pool.cpp
	src/main.cpp
	src/request_handler.cpp
	src/request_parser.cpp
	src/reply.cpp
	src/server.cpp
)

add_executable(hydra ${HEADER_FILES} ${SOURCE_FILES} ${CGI_HEADER_FILES} ${CGI_SOURCE_FILES})

target_link_libraries(hydra ${HYDRA_LIBS})

INSTALL(TARGETS hydra DESTINATION ${PROJECT_SOURCE_DIR}/bin)
