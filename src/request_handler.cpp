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

namespace Hydra {

request_handler::request_handler(Server* server) : m_server(server){

}

bool isHost(const header h){
	return (h.name == "Host");
}

void request_handler::handle_request(const request& req, reply& rep){

	// Hydra Functionality.

	std::vector<header>::const_iterator it = std::find_if(req.headers.begin(), req.headers.end(), isHost);

	if(it == req.headers.end()){
		std::cerr << "Hydra: No Host Header" << std::endl;
		rep = reply::stock_reply(reply::service_unavailable);
		return;
	}

	Host* host = m_server->host(it->value);

	if(host == NULL){
		rep = reply::stock_reply(reply::service_unavailable);
		return;
	} else {
		host->request(req, rep);
		return;
	}

}

bool request_handler::url_decode(const std::string& in, std::string& out){
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i){
		if (in[i] == '%'){
			if (i + 3 <= in.size()){
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value){
					out += static_cast<char>(value);
					i += 2;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else if (in[i] == '+'){
			out += ' ';
		} else {
			out += in[i];
		}
	}
	return true;
}

}
