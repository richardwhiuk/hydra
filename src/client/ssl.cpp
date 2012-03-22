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
#include "log.hpp"

#include <boost/bind.hpp>

Hydra::Client::SSL::SSL(std::string name, Config::Section config, Daemon& hydra) : HTTP(name, config, hydra), m_context(0){

}

Hydra::Client::SSL::~SSL(){

}

void Hydra::Client::SSL::run(boost::asio::io_service& io_service){

	m_context = new boost::asio::ssl::context(io_service, boost::asio::ssl::context::sslv23);

	m_context->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);

	m_context->use_certificate_file(m_config.value("certificate").c_str(), boost::asio::ssl::context::pem);

	m_context->use_certificate_chain_file(m_config.value("chain").c_str());

	m_context->use_rsa_private_key_file(m_config.value("private").c_str(), boost::asio::ssl::context::pem);

	HTTP::run(io_service);

}

void Hydra::Client::SSL::accept(){

	Client::SSL::Connection::pointer connect = Client::SSL::Connection::Create(m_accept->get_io_service(), m_hydra, *m_context, m_tag);

	m_accept->async_accept(connect->socket().base(), boost::bind(
		&Client::SSL::handle, 
		this, 
		connect, 
		boost::asio::placeholders::error));

}

Hydra::Client::SSL::Connection::Connection(boost::asio::io_service& io_service, Daemon& hydra, boost::asio::ssl::context& context, std::string& tag) : HTTP::Connection(io_service, hydra, tag), m_socket(io_service, context){

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

	timeout(boost::posix_time::seconds(m_read_timeout));

	m_socket.async_handshake(
		boost::asio::ssl::stream_base::server,
		boost::bind(&Client::SSL::Connection::handle_handshake, shared_from_this(), boost::asio::placeholders::error)
	);

}

void Hydra::Client::SSL::Connection::handle_handshake(const boost::system::error_code& e){

	if(!e){

		m_timer.cancel();

		begin();

	}

}

Hydra::Client::SSL::Socket& Hydra::Client::SSL::Connection::socket(){

	return m_socket;

}

