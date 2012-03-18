//
// ssl.cpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "client/ssl.hpp"
#include "connection.hpp"
#include "daemon.hpp"

#include <boost/bind.hpp>

Hydra::Client::SSL::SSL(std::string name, Config::Section config, Daemon& hydra) : Base(name, config, hydra), m_accept(0), m_context(0){

}

Hydra::Client::SSL::~SSL(){
	if(m_accept)
		delete m_accept;

	if(m_context)
		delete m_context;
}

void Hydra::Client::SSL::run(boost::asio::io_service& io_service){

	m_context = new boost::asio::ssl::context(io_service, boost::asio::ssl::context::sslv23);

	m_context->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);

	m_context->use_certificate_file(m_config.value("certificate").c_str(), boost::asio::ssl::context::pem);

	m_context->use_certificate_chain_file(m_config.value("chain").c_str());

	m_context->use_rsa_private_key_file(m_config.value("private").c_str(), boost::asio::ssl::context::pem);

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

void Hydra::Client::SSL::accept(){

	Client::SSL::Connection::pointer connect = Client::SSL::Connection::Create(m_accept->get_io_service(), m_hydra, *m_context, m_tag);

	m_accept->async_accept(connect->socket(), boost::bind(
		&Client::SSL::handle, 
		this, 
		connect, 
		boost::asio::placeholders::error));

}

void Hydra::Client::SSL::handle(Connection::pointer connect, const boost::system::error_code& error){

	if(!error){
		
		connect->start();

	}

	accept();

}

Hydra::Client::SSL::Connection::Connection(boost::asio::io_service& io_service, Daemon& hydra, boost::asio::ssl::context& context, std::string& tag) : m_hydra(hydra), m_bytes_start(0), m_bytes_total(0), m_socket(io_service, context), m_tag(tag){

}

Hydra::Client::SSL::Connection::~Connection(){

}

Hydra::Client::SSL::Connection::pointer Hydra::Client::SSL::Connection::Create(boost::asio::io_service& io_service, Daemon& hydra, boost::asio::ssl::context& context, std::string& tag){
	return pointer(new Connection(io_service, hydra, context, tag));
}

void Hydra::Client::SSL::Connection::start(){

	handshake();

}

void Hydra::Client::SSL::Connection::handshake(){

	m_socket.async_handshake(
		boost::asio::ssl::stream_base::server,
		boost::bind(&Client::SSL::Connection::handle_handshake, shared_from_this(), boost::asio::placeholders::error)
	);

}

void Hydra::Client::SSL::Connection::handle_handshake(const boost::system::error_code& e){

	if(!e){

		begin();

	}

}

void Hydra::Client::SSL::Connection::begin(){

	m_connection = Hydra::Connection::Create(m_tag);
	
	m_connection->response().bind_write(boost::bind(&Client::SSL::Connection::write, shared_from_this()));
	m_connection->response().bind_finish(boost::bind(&Client::SSL::Connection::finish, shared_from_this()));

	m_persistent = false;

	if(m_bytes_total > m_bytes_start){

		consume();

	} else {
		
		read();

	}

}

void Hydra::Client::SSL::Connection::read(){

	// Async read.

	m_socket.async_read_some(
		boost::asio::buffer(m_buffer_in), 
		boost::bind(&Client::SSL::Connection::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::SSL::Connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_bytes_start = 0;
		m_bytes_total = bytes_transferred;

		consume();

	}

}

void Hydra::Client::SSL::Connection::consume(){

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
		header += ", " + m_socket.lowest_layer().remote_endpoint().address().to_string();

	} catch(Exception* e){

		delete e;

		try {

			m_connection->request().header("X-Forward-For", m_socket.lowest_layer().remote_endpoint().address().to_string());

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

void Hydra::Client::SSL::Connection::write_start(){

	// Modify headers

	if(m_persistent){

		m_connection->response().header("Connection", "Keep-Alive");

	} else {

		m_connection->response().header("Connection", "close");

	}

	m_connection->response().bind_write(boost::bind(&Client::SSL::Connection::write, shared_from_this()));

	write();

}

void Hydra::Client::SSL::Connection::write(){

	m_connection->response().read_buffer(m_buffer_out);

	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(m_buffer_out),
		boost::bind(&Client::SSL::Connection::handle_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::SSL::Connection::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred){

	if(!e){

		m_connection->response().read();

	}

}

void Hydra::Client::SSL::Connection::finish(){

	m_connection->response().read_buffer(m_buffer_out);

	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(m_buffer_out),
		boost::bind(&Client::SSL::Connection::handle_finish, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);

}

void Hydra::Client::SSL::Connection::handle_finish(const boost::system::error_code& e, std::size_t bytes_transferred){

	m_connection.reset();

	if(m_persistent){

		begin();

	}

}

boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type& Hydra::Client::SSL::Connection::socket() {

	return m_socket.lowest_layer();

}

