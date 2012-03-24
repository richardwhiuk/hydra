//
// http.cpp
// ~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "client/http.hpp"
#include "connection.hpp"
#include "daemon.hpp"
#include "log.hpp"

#include <boost/bind.hpp>

Hydra::Client::HTTP::HTTP(std::string name, Config::Section config, Daemon& hydra) : Base(name, config, hydra), m_accept(0){

}

void Hydra::Client::HTTP::run(boost::asio::io_service& io_service){

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

void Hydra::Client::HTTP::handle(Connection::pointer connect, const boost::system::error_code& error){

	if(!error){

		connect->start();

	}

	accept();

}

Hydra::Client::HTTP::Connection::Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag) : m_hydra(hydra), m_tag(tag), m_timer(io_service), m_read_timeout(10), m_write_timeout(10), m_bytes_start(0), m_bytes_total(0){

}

Hydra::Client::HTTP::Connection::~Connection(){

}

void Hydra::Client::HTTP::Connection::timeout(boost::posix_time::time_duration time){

	// Prevent Async Timeout

	m_timer.expires_from_now(time);

	m_timer.async_wait(
		boost::bind(
			&Client::HTTP::Connection::handle_timeout,
			boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this(),
			boost::asio::placeholders::error
		)
	);

}

void Hydra::Client::HTTP::Connection::handle_timeout(const boost::system::error_code& e){

	if (e != boost::asio::error::operation_aborted){

		// Close socket with prejudice.

		socket().cancel();
		socket().close();

	}


}

void Hydra::Client::HTTP::Connection::start(){

	begin();

}

void Hydra::Client::HTTP::Connection::begin(){

	m_connection = Hydra::Connection::Create(m_tag);
	m_connection->response().bind_write(boost::bind(&Client::HTTP::Connection::write_start, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this()));
	m_connection->response().bind_finish(boost::bind(&Client::HTTP::Connection::finish, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this()));
	
	m_persistent = false;

	m_address = socket().remote_address();

	if(m_bytes_total > m_bytes_start){
		consume();
	} else {
		read();
	}

}

void Hydra::Client::HTTP::Connection::read(){

	// Async read.

	timeout(boost::posix_time::seconds(m_read_timeout));

	socket().async_read(
		boost::asio::buffer(m_buffer_in), 
		boost::asio::transfer_at_least(1),			
		boost::bind(&Client::HTTP::Connection::handle_read, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::HTTP::Connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_timer.cancel();

		m_bytes_start = 0;
		m_bytes_total = bytes_transferred;

		consume();

	} else {

		m_connection.reset();

	}

}

void Hydra::Client::HTTP::Connection::consume(){

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
		header += ", " + m_address;

	} catch(Exception* e){

		delete e;

		try {

			m_connection->request().header("X-Forward-For", m_address);

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

void Hydra::Client::HTTP::Connection::write_start(){

	// Modify headers

	if(m_persistent){

		m_connection->response().header("Connection", "Keep-Alive");

	} else {

		m_connection->response().header("Connection", "close");

	}

	m_connection->response().bind_write(boost::bind(&Client::HTTP::Connection::write, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this()));

	write();

}

void Hydra::Client::HTTP::Connection::write(){

	m_connection->response().read_buffer(m_buffer_out);

	// Async write

	timeout(boost::posix_time::seconds(m_write_timeout));

	socket().async_write(
		boost::asio::buffer(m_buffer_out),
		boost::asio::transfer_at_least(1),			
		boost::bind(&Client::HTTP::Connection::handle_write, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::HTTP::Connection::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_timer.cancel();

		m_connection->response().read();

	} else {

		m_connection.reset();

	}

}

void Hydra::Client::HTTP::Connection::finish(){

	m_connection->response().read_buffer(m_buffer_out);

	// Async write

	timeout(boost::posix_time::seconds(m_write_timeout));

	socket().async_write(
		boost::asio::buffer(m_buffer_out),
		boost::bind(&Client::HTTP::Connection::handle_finish, boost::enable_shared_from_this<Hydra::Client::HTTP::Connection>::shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::HTTP::Connection::handle_finish(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_timer.cancel();

		{
			boost::shared_ptr<Hydra::Log> access = Log::access();

			(*access) 	<< "[" << m_address
					<< "] [" << m_tag
					<< "] [" << m_connection->request().method()
					<< "] [";

			try {
		
				(*access) << m_connection->request().header("Host");

			} catch(Exception* e){

				delete e;

			}
		
			(*access)	<< "] [" << m_connection->request().path()
					<< "] [" << m_connection->response().code()
					<< "]" << std::endl;

		}
	
		m_connection.reset();
	
		if(m_persistent){

			start();		
	
		}

	} else {

		m_connection.reset();

	}

}



