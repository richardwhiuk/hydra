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
#include "log.hpp"

#include <boost/bind.hpp>

Hydra::Client::Plain::Plain(std::string name, Config::Section config, Daemon& hydra) : HTTP(name, config, hydra){

}

Hydra::Client::Plain::~Plain(){

	if(m_accept)
		delete m_accept;

}

void Hydra::Client::Plain::accept(){

	Client::Plain::Connection::pointer connect = Client::Plain::Connection::Create(m_accept->get_io_service(), m_hydra, m_tag);

	m_accept->async_accept(connect->socket().base(), boost::bind(
		&Client::Plain::handle, 
		this, 
		connect, 
		boost::asio::placeholders::error));

}

Hydra::Client::Plain::Connection::Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag) : HTTP::Connection(io_service, hydra, tag), m_socket(io_service){

}

Hydra::Client::Plain::Connection::~Connection(){

}

Hydra::Client::Plain::Connection::pointer Hydra::Client::Plain::Connection::Create(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag){
	return pointer(new Connection(io_service, hydra, tag));
}

Hydra::Client::Plain::Socket& Hydra::Client::Plain::Connection::socket(){
	return m_socket;
}

