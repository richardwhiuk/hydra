//
// request.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request.hpp"

#include <sstream>

Hydra::Request::Request() : m_state(START){

}

const std::string& Hydra::Request::path(){
	return m_path;
}

void Hydra::Request::header(const std::string& key, const std::string& value){

	m_headers[key] = value;

}

std::string Hydra::Request::header(const std::string& key){

	std::map<std::string, std::string>::iterator it = m_headers.find(key);

	if(it == m_headers.end()){

		throw new Exception(std::string("Hydra->Request->Header ").append(key).append(" not found."));

	}

	return it->second;

}

void Hydra::Request::read_buffer(std::string& buffer){

	std::stringstream ss;

	ss << m_method << ' ' << m_path << " HTTP/" << m_version << "\r\n";

	for(std::map<std::string, std::string>::iterator it = m_headers.begin(); it != m_headers.end(); ++it){

		ss << it->first << ": " << it->second << "\r\n";

	}

	ss << "\r\n";

	buffer = ss.str();

}

