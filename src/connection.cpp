//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"

Hydra::Connection::Connection(std::string tag) : m_request(), m_response(), m_tag(tag){
	
}

Hydra::Connection::~Connection(){
	
}

Hydra::Connection::pointer Hydra::Connection::Create(std::string tag){
	return pointer(new Connection(tag));
}

Hydra::Request& Hydra::Connection::request(){
	return m_request;
}

Hydra::Response& Hydra::Connection::response(){
	return m_response;
}

const std::string& Hydra::Connection::tag(){
	return m_tag;
}

