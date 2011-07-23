//
// host.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include "host.hpp"

Hydra::Host::Host(Config::Section details, Server* server) : m_details(details), m_engine(NULL), m_server(server) {

}

Hydra::Host::~Host(){
	if(m_engine != NULL){
		delete m_engine;
		m_engine = NULL;
	}
}

bool Hydra::Host::valid(){
	std::string name = m_details.name();

	if( m_details["engine"].size() == 0){
		return false;	
	}

	if( ! Engine::Valid(m_details["engine"].front())){
		return false;
	}

	if( (name.length() > 0) && name != "global" ){
		return true;
	}

	return false;
}

std::vector<std::string> Hydra::Host::alias(){
	Config::Section::iterator it = m_details.find("alias");
	if(it != m_details.end()){
		return it->second;
	} else {
		return std::vector<std::string>();
	}	
}

void Hydra::Host::request(const Hydra::request& req, Hydra::reply& rep){

	if(m_engine == NULL){
		m_engine_mux.lock();
		if(m_engine == NULL){
			Engine* engine;
			engine = Engine::Create(m_details["engine"].front(), m_details, m_server);
			m_engine = engine;
		}
		m_engine_mux.unlock();
	}

	m_engine->request(req, rep);

/*	// Decode url to path.
	std::string request_path;

	if (!url_decode(req.uri, request_path)){
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	// Request path must be absolute and not contain "..".
	if (request_path.empty() || request_path[0] != '/'
			|| request_path.find("..") != std::string::npos)
	{
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	// If path ends in slash (i.e. is a directory) then add "index.html".
	if (request_path[request_path.size() - 1] == '/')
	{
		request_path += "index.html";
	}

	// Determine the file extension.
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");
	std::string extension;
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
	{
		extension = request_path.substr(last_dot_pos + 1);
	}

	// Open the file to send back.
	std::string full_path = doc_root_ + request_path;
	std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
	if (!is){
		rep = reply::stock_reply(reply::not_found);
		return;
	}

	// Fill out the reply to be sent to the client.
	rep.status = reply::ok;
	char buf[512];
	while (is.read(buf, sizeof(buf)).gcount() > 0)
		rep.content.append(buf, is.gcount());
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
	rep.headers[1].name = "Content-Type";
	rep.headers[1].value = mime_types::extension_to_type(extension);
*/

}

