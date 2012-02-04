//
// plain.cpp
// ~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "client/plain.hpp"
#include "connection.hpp"
#include "daemon.hpp"

#include <boost/bind.hpp>

Hydra::Client::Plain::Plain(std::string name, Config::Section config, Daemon& hydra) : Base(name, config, hydra), m_accept(0){

}

Hydra::Client::Plain::~Plain(){

	if(m_accept)
		delete m_accept;

}

void Hydra::Client::Plain::run(boost::asio::io_service& io_service){

	m_tag = m_config.value("tag");

	std::stringstream pstr(m_config.value("port"));

	unsigned short port;

	pstr >> port;

	boost::asio::ip::tcp::endpoint* endpoint = 0;

	if(m_config.value("address") == "any"){
		if(m_config.value("protocol") == "v4"){
			endpoint = new boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port );
		} else if(m_config.value("protocol") == "v6"){
			endpoint = new boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v6(), port );
		}
	} else {
		endpoint = new boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( m_config.value("address") ), port );
	}

	if(endpoint){
	
		m_accept = new boost::asio::ip::tcp::acceptor(
				io_service, 
				*endpoint
			);

		accept();

	}

}

void Hydra::Client::Plain::accept(){

	Client::Plain::Connection::pointer connect = Client::Plain::Connection::Create(m_accept->get_io_service(), m_hydra, m_tag);

	m_accept->async_accept(connect->socket(), boost::bind(
		&Client::Plain::handle, 
		this, 
		connect, 
		boost::asio::placeholders::error));

}

void Hydra::Client::Plain::handle(Connection::pointer connect, const boost::system::error_code& error){

	if(!error){

		connect->start();

	}

	accept();

}

Hydra::Client::Plain::Connection::Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag) : m_hydra(hydra), m_bytes_start(0), m_bytes_total(0), m_tag(tag), m_socket(io_service){

}

Hydra::Client::Plain::Connection::~Connection(){

}

Hydra::Client::Plain::Connection::pointer Hydra::Client::Plain::Connection::Create(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag){
	return pointer(new Connection(io_service, hydra, tag));
}

void Hydra::Client::Plain::Connection::start(){

	m_connection = Hydra::Connection::Create(m_tag);
	m_connection->response().bind_write(boost::bind(&Client::Plain::Connection::write_start, shared_from_this()));
	m_connection->response().bind_finish(boost::bind(&Client::Plain::Connection::finish, shared_from_this()));
	
	m_persistent = false;

	if(m_bytes_total > m_bytes_start){
		consume();
	} else {
		read();
	}

}

void Hydra::Client::Plain::Connection::read(){

	// Async read.

	m_socket.async_read_some(
		boost::asio::buffer(m_buffer_in), 
		boost::bind(&Client::Plain::Connection::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::Plain::Connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_bytes_start = 0;
		m_bytes_total = bytes_transferred;

		consume();

	} else {

		m_connection.reset();

	}

}

void Hydra::Client::Plain::Connection::consume(){

	try {
	
		// Parse the request.

		bool done = m_connection->request().write_buffer(m_buffer_in, m_bytes_start, m_bytes_total);

		if(!done){
			read();
			return;
		}

	} catch(Exception* e){

		m_connection->response().error(400);

		return;

	}

	// We have a complete request.

	// Should we persist the connection?

	try {

		if(
			   (m_connection->request().header("Connection") == "Keep-Alive")
			|| (m_connection->request().header("Connection") == "Keep-alive")
			|| (m_connection->request().header("Connection") == "keep-Alive")
			|| (m_connection->request().header("Connection") == "keep-alive")
		){

			m_persistent = true;

		} else {

			m_persistent = false;			

		}

	} catch(Exception* e){

		delete e;

		if(m_connection->request().version() == "1.1"){

			m_persistent = true;

		} else {

			m_persistent = false;

		}

	}


	// Add Proxy Headers

	try {
		std::string header = m_connection->request().header("X-Forwarded-For");
		header += ", " + m_socket.remote_endpoint().address().to_string();

	} catch(Exception* e){

		delete e;

		try {

			m_connection->request().header("X-Forward-For", m_socket.remote_endpoint().address().to_string());

		} catch(Exception* e){

			delete e;

			m_connection->response().error(400);

		}

	}

	// Handle Connection

	try {

		m_hydra.handle(m_connection);

	} catch(Exception* e){

		delete e;

		m_connection->response().error(404);

	}

}

void Hydra::Client::Plain::Connection::write_start(){

	// Modify headers

	if(m_persistent){

		m_connection->response().header("Connection", "Keep-Alive");

	} else {

		m_connection->response().header("Connection", "close");

	}

	m_connection->response().bind_write(boost::bind(&Client::Plain::Connection::write_start, shared_from_this()));

	write();

}

void Hydra::Client::Plain::Connection::write(){

	m_connection->response().read_buffer(m_buffer_out);

	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(m_buffer_out),
		boost::bind(&Client::Plain::Connection::handle_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::Plain::Connection::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_connection->response().read();

	}

}

boost::asio::ip::tcp::socket& Hydra::Client::Plain::Connection::socket(){
	return m_socket;
}

void Hydra::Client::Plain::Connection::finish(){

	m_connection->response().read_buffer(m_buffer_out);

	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(m_buffer_out),
		boost::bind(&Client::Plain::Connection::handle_finish, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::Plain::Connection::handle_finish(const boost::system::error_code& e, std::size_t bytes_transferred){

	m_connection.reset();

	if(m_persistent){

		start();		

	}


}

