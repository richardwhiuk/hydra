//
// proxy.cpp
// ~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server/proxy.hpp"

#include <boost/bind.hpp>

Hydra::Server::Proxy::Proxy(std::string name, Hydra::Config::Section config, Daemon& hydra) : Base(name, config, hydra){

}

Hydra::Server::Proxy::~Proxy(){

}

void Hydra::Server::Proxy::run(boost::asio::io_service& io_service){

	m_io_service = &io_service;

}

void Hydra::Server::Proxy::handle(Hydra::Connection::pointer connection){

	Proxy::Connection::pointer connect = Proxy::Connection::Create(*m_io_service, m_config);

	connect->start(connection);

}

Hydra::Server::Proxy::Connection::pointer Hydra::Server::Proxy::Connection::Create(boost::asio::io_service& io_service, Config::Section& config){

	return pointer(new Connection(io_service, config));

}

Hydra::Server::Proxy::Connection::Connection(boost::asio::io_service& io_service, Config::Section& config) : m_config(config), m_resolver(io_service), m_socket(io_service){

}

Hydra::Server::Proxy::Connection::~Connection(){

}

void Hydra::Server::Proxy::Connection::start(Hydra::Connection::pointer connection){

	m_connection = connection;

	// For now, just resolve. In the future, with connection reuse, we may be able to be more clever here.

	resolve();

}

void Hydra::Server::Proxy::Connection::resolve(){

	/*std::stringstream pstr(m_config.value("port"));

	int port;

	psft >> port;*/

	boost::asio::ip::tcp::resolver::query query(m_config.value_tag("server", m_connection->tag()), m_config.value_tag("port", m_connection->tag()));

	m_resolver.async_resolve(query, boost::bind(&Connection::handle_resolve, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::iterator));

}

void Hydra::Server::Proxy::Connection::handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint){

	if(!err){

		connect(endpoint);

	} else {

		// Failed to resolve address.

		m_connection->response().error(503);

		m_connection.reset();

	}

}

void Hydra::Server::Proxy::Connection::connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator){

	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

	m_socket.async_connect(endpoint, boost::bind(&Connection::handle_connect, shared_from_this(), boost::asio::placeholders::error, ++endpoint_iterator));

}

void Hydra::Server::Proxy::Connection::handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator){

	if(!err){

		write();

	} else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {

		m_socket.close();

		connect(endpoint_iterator);

	} else {

		m_connection->response().error(504);

		m_connection.reset();

	}

}

void Hydra::Server::Proxy::Connection::write(){

	Hydra::Request req = m_connection->request();

	req.header("Connection","close");

	req.read_buffer(m_buffer_out);

	boost::asio::async_write(m_socket, boost::asio::buffer(m_buffer_out), boost::bind(&Connection::handle_write, shared_from_this(), boost::asio::placeholders::error));

}

void Hydra::Server::Proxy::Connection::handle_write(const boost::system::error_code& err){

	if(!err){

		read();

	} else {

		m_connection->response().error(502);

		m_connection.reset();

	}

}

void Hydra::Server::Proxy::Connection::read(){

	boost::asio::async_read(m_socket, boost::asio::buffer(m_buffer_in), boost::asio::transfer_at_least(1), boost::bind(&Connection::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

}

void Hydra::Server::Proxy::Connection::handle_read(const boost::system::error_code& err, std::size_t bytes){

	if(!err){

		m_connection->response().write_buffer(m_buffer_in, bytes);

		m_connection->response().bind_read(boost::bind(&Connection::read, shared_from_this()));

		m_connection->response().write();

	} else if(err != boost::asio::error::eof) {

		// Error

		m_connection->response().done();

		m_connection.reset();

	} else {

		// EOF

		m_connection->response().done();

		m_connection.reset();

	}

}

