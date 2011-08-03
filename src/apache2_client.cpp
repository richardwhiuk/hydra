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
#include "apache2.hpp"

Hydra::Apache2::Client::Client(const std::string& server, const std::string& port) : m_server(server), m_port(port){

}

void Hydra::Apache2::Client::run(Hydra::Connection& ccon){
	
	boost::asio::io_service ios;

	Apache2::Client::Connection con(ios, ccon, *this);

	ios.run();

}

Hydra::Apache2::Client::~Client(){

}

const std::string& Hydra::Apache2::Client::port(){
	return m_port;
}

const std::string& Hydra::Apache2::Client::server(){
	return m_server;
}

