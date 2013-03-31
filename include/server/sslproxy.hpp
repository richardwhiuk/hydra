//
// sslproxy.hpp
// ~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_SERVER_SSLPROXY_HPP
#define HYDRA_FILE_SERVER_SSLPROXY_HPP

#include "server.hpp"
#include "config.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Hydra {

class Daemon;

namespace Server {

class SslProxy : public Base {

public:

	class Connection : public boost::enable_shared_from_this<Connection> {

	public:

		typedef boost::shared_ptr<Connection> pointer;

		static Connection::pointer Create(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Config::Section& config);
		~Connection();

		void start(Hydra::Connection::pointer connection);

	private:

		Connection(boost::asio::io_service& io_service, boost::asio::ssl::context& context, Config::Section& config);

		void resolve();

		void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		void connect(boost::asio::ip::tcp::resolver::iterator endpoint);

		void handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		void handshake();

		void handle_handshake(const boost::system::error_code& err);

		void write();

		void handle_write(const boost::system::error_code& err);

		void read();

		void handle_read(const boost::system::error_code& err, size_t bytes);

		Config::Section& m_config;

		Hydra::Connection::pointer m_connection;

		boost::asio::ip::tcp::resolver m_resolver;
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket;

		std::string m_buffer_out; 

		boost::array<char, 8192> m_buffer_in;

	};

	SslProxy(std::string name, Config::Section config, Config::Section defaults, Daemon& hydra);
	virtual ~SslProxy();

	virtual void run(boost::asio::io_service& io_service);

	virtual void handle(Hydra::Connection::pointer connection);

private:

	boost::asio::io_service* m_io_service;
	boost::asio::ssl::context* m_context;

};

}

}

#endif

