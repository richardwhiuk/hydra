//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "reply.hpp"
#include "stock_replies.hpp"
#include "status_strings.hpp"
#include "misc_strings.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include <exception>

Hydra::Reply::Reply() : state(Reply::none){

}

Hydra::Reply::~Reply(){

	for(std::vector<std::string*>::iterator it = sending.begin(); it != sending.end(); ++it){
		delete (*it);
	}
	sending.clear();

	std::size_t size = tosend.size();

	while(size > 0){
		delete tosend.front();
		tosend.pop();
		--size;
	}

}

Hydra::Reply& Hydra::Reply::operator=(const Reply& rhs){
	if(this == &rhs){
		return *this;
	}

	if(state != Reply::none || sending.size() > 0 || rhs.sending.size() > 0){
		throw std::runtime_error("Invalid Reply Assignment");
	}

	headers = rhs.headers;

	std::size_t size = tosend.size();

	while(size > 0){
		delete tosend.front();
		tosend.pop();
		--size;
	}

	std::queue<std::string*> nts = rhs.tosend;

	size = nts.size();
	
	while(size > 0){
		tosend.push(nts.front());
		nts.pop();
		--size;
	}

	state = rhs.state;
	status = rhs.status;

	return *this;
}

Hydra::Reply::Reply(const Reply& rhs){
	this->operator=(rhs);
}

void Hydra::Reply::content(std::string content){
	std::string* str = new std::string(content);
	m_tosend_mux.lock();
	tosend.push(str);
	m_tosend_mux.unlock();
}

void Hydra::Reply::discard(){
	m_sending_mux.lock();
	for(std::vector<std::string*>::iterator it = sending.begin(); it != sending.end(); ++it){
		delete (*it);
	}
	sending.clear();
	m_sending_mux.unlock();
}

std::vector<boost::asio::const_buffer> Hydra::Reply::buffers(){

	std::vector<boost::asio::const_buffer> buffers;

	if(state == Reply::none){

		buffers.push_back(status_strings::to_buffer(status));

		for (std::size_t i = 0; i < headers.size(); ++i){
	
			Header& h = headers[i];
			buffers.push_back(boost::asio::buffer(h.name));
			buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
			buffers.push_back(boost::asio::buffer(h.value));
			buffers.push_back(boost::asio::buffer(misc_strings::crlf));
		}

		buffers.push_back(boost::asio::buffer(misc_strings::crlf));

		state = header;

	}

	if(state == Reply::header || state == Reply::partial){

		m_sending_mux.lock();		// This shouldn't be needed. Oh well.
		m_tosend_mux.lock();

		std::size_t strings = tosend.size();

		if(strings == 0){
			m_tosend_mux.unlock();
			m_sending_mux.unlock();
			return buffers;
		}

		for(; strings > 0; --strings){
			sending.push_back(tosend.front());
			tosend.pop();
		}

		m_tosend_mux.unlock();

		for(std::vector<std::string*>::iterator it = sending.begin(); it != sending.end(); ++it){
			buffers.push_back(boost::asio::buffer(*(*it)));
		}
	
		state = Reply::partial;

		m_sending_mux.unlock();

	}	

	return buffers;
}

Hydra::Reply Hydra::Reply::Stock(Reply::status_type status)
{
	Reply rep;
	rep.status = status;
	std::string content = stock_replies::to_string(status);
	rep.content(content);
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = boost::lexical_cast<std::string>(content.size());
	rep.headers[1].name = "Content-Type";
	rep.headers[1].value = "text/html";
	return rep;
}


