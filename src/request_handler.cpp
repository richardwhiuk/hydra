//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "server.hpp"

Hydra::Request_Handler::Request_Handler(Server* server) : m_server(server){

}

bool isHost(const Hydra::Header h){
	return (h.name == "Host");
}

void Hydra::Request_Handler::handle_request(Hydra::Connection::Ptr con){

	// Hydra Functionality.

	std::vector<Header>::const_iterator it = std::find_if(con->request().headers.begin(), con->request().headers.end(), isHost);

	if(it == con->request().headers.end()){
		std::cerr << "Hydra: No Host Header" << std::endl;
		con->reply().stock(Reply::service_unavailable);
		return;
	}

	Host* host = m_server->host(it->value);

	if(host == NULL){
		con->reply().stock(Reply::service_unavailable);
		return;
	} else {
		host->request(con);
		return;
	}

}

