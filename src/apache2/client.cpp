//
// apache2-client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <sstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <apache2/connection.hpp>
#include <apache2/client.hpp>
#include <apache2/engine.hpp>

Hydra::Apache2::Client::Client(Hydra::Apache2::Engine& engine) : m_engine(engine){

}

bool Hydra::Apache2::Client::ready(){
	if(m_engine.port().size() == 0)
		return false;
	if(m_engine.address().size() == 0)
		return false;

	return true;
}

Hydra::Apache2::Connection::Ptr Hydra::Apache2::Client::connection(){
	// In the future we should reuse old connections. 

	// For now however, we will create a new one.

	Hydra::Apache2::Connection::Ptr con = Hydra::Apache2::Connection::Ptr(
		new Hydra::Apache2::Connection(m_engine.server().io_service(), m_engine, *this));

	con->init();

	return con;

}

Hydra::Apache2::Client::~Client(){

}

const std::string& Hydra::Apache2::Client::port(){
	return m_engine.port().front();
}

const std::string& Hydra::Apache2::Client::server(){
	return m_engine.address().front();
}

