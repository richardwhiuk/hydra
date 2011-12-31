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

long Hydra::Connection::id = 0;

Hydra::Connection::Connection() : m_id(++id), m_request(m_id), m_response(m_id){
	
}

Hydra::Connection::~Connection(){
	
}

Hydra::Connection::pointer Hydra::Connection::Create(){
	return pointer(new Connection());
}

Hydra::Request& Hydra::Connection::request(){
	return m_request;
}

Hydra::Response& Hydra::Connection::response(){
	return m_response;
}

