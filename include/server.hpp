//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_SERVER_HPP
#define HYDRA_FILE_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "io_service_pool.hpp"
#include "request_handler.hpp"
#include "config.hpp"
#include "version.hpp"
#include "host.hpp"

namespace Hydra {

class Server : private boost::noncopyable {

public:

	Server(std::string& config) : m_setup(false), m_config(config), m_request_handler(this){
	
	}

	bool setup();

	bool setup_hosts();

	Host* host(std::string host);
	
	void run();

	unsigned int go();

	void stop();

	~Server(){

	}

private:

	/// Handle completion of an asynchronous accept operation.
	void handle_accept(const boost::system::error_code& e);

	bool m_setup;

	/// Configuration Hosts
	std::map<std::string, Host> m_configs;

	/// Hosts
	std::map<std::string, Host*> m_hosts;

	/// Hydra Configuration
	Hydra::Config m_config;

	/// The pool of io_service objects used to perform asynchronous operations.
	Hydra::io_service_pool_ptr m_io_service_pool;

	/// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor* m_acceptor;

	/// The next connection to be accepted.
	Hydra::connection_ptr m_new_connection;

	/// The handler for all incoming requests.
	Hydra::request_handler m_request_handler;

};

}

#endif

