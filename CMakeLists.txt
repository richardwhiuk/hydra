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

set (HYDRA_VERSION 0.2.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(OpenSSL REQUIRED)

find_package(Boost COMPONENTS thread system REQUIRED)

set (HYDRA_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${OPENSSL_LIBRARIES})

configure_file (
  "${PROJECT_SOURCE_DIR}/include/version.hpp.in"
  "${PROJECT_BINARY_DIR}/include/version.hpp"
  )

include_directories("include" "${PROJECT_BINARY_DIR}/include")

# Core

set(HEADER_FILES
	include/config.hpp
	include/connection.hpp
	include/client.hpp
	include/client/http.hpp
	include/client/plain.hpp
	include/client/ssl.hpp
	include/daemon.hpp
	include/exception.hpp
	include/hostmap.hpp
	include/http.hpp
	include/log.hpp
	include/request.hpp
	include/response.hpp
	include/server.hpp
	include/server/apache2.hpp
	include/server/nginx.hpp
	include/server/proxy.hpp
	include/server/sslproxy.hpp
	include/server/redirect.hpp
	include/utility.hpp
	include/version.hpp
)

set(SOURCE_FILES
	src/config.cpp
	src/connection.cpp
	src/client.cpp
	src/client/http.cpp
	src/client/plain.cpp
	src/client/ssl.cpp
	src/daemon.cpp
	src/exception.cpp
	src/hostmap.cpp
	src/http.cpp
	src/log.cpp
	src/main.cpp
	src/request.cpp
	src/response.cpp
	src/server.cpp
	src/server/apache2.cpp
	src/server/nginx.cpp
	src/server/proxy.cpp
	src/server/sslproxy.cpp
	src/server/redirect.cpp
	src/utility.cpp
)

add_executable(hydra ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(hydra ${HYDRA_LIBS})

INSTALL(TARGETS hydra DESTINATION /usr/local/sbin)

