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
#include <iostream>

Hydra::Reply::Reply() : m_in_state(Reply::state_none), m_out_state(Reply::state_none), m_content_added(false), m_q_data(false){

}

Hydra::Reply::~Reply(){

	for(std::vector<std::string*>::iterator it = m_out.begin(); it != m_out.end(); ++it){
		delete (*it);
	}

	std::size_t size = m_in.size();

	while(size > 0){
		delete m_in.front();
		m_in.pop();
		--size;
	}

}

// In

void Hydra::Reply::status(status_type status){
	m_status = status;
	{
		boost::lock_guard<boost::mutex> in_lock(m_in_mutex);
		m_in_state = state_status;
	}
	m_in_cond.notify_one();
}

void Hydra::Reply::header(Hydra::Header& header){
	m_headers.push_back(header);
}

void Hydra::Reply::headers_complete(){
	{
		boost::lock_guard<boost::mutex> in_lock(m_in_mutex);
		m_in_state = state_header;
	}
	m_in_cond.notify_one();
	
}

void Hydra::Reply::content(std::string content){
	std::string* str = new std::string(content);

	if(m_content_added){
		{
			boost::lock_guard<boost::mutex> in_lock(m_in_mutex);
			m_in_state = state_partial;
		}
		m_content_added = true;
	}
	
	m_in_cond.notify_one();

	{
		boost::lock_guard<boost::mutex> q_lock(m_q_mutex);
		m_q_data = true;
		m_in.push(str);
	}
	m_q_cond.notify_one();
}

void Hydra::Reply::finish(){
	
	{
		boost::lock_guard<boost::mutex> in_lock(m_in_mutex);
		m_in_state = state_done;
	}

	m_in_cond.notify_one();

}

// Stock Reply

void Hydra::Reply::stock(status_type status){
	m_status = status;
	std::string* content = new std::string(stock_replies::to_string(status));

	m_headers.resize(2);
	m_headers[0].name = "Content-Length";
	m_headers[0].value = boost::lexical_cast<std::string>(content->size());
	m_headers[1].name = "Content-Type";
	m_headers[1].value = "text/html";
	m_content_added = true;

	{
		boost::lock_guard<boost::mutex> in_lock(m_in_mutex);
		m_in_state = state_done;
	}

	m_in_cond.notify_one();

	{
		boost::lock_guard<boost::mutex> q_lock(m_q_mutex);
		m_q_data = true;
		m_in.push(content);
	}

	m_q_cond.notify_one();

}

// Out

bool Hydra::Reply::buffer(std::vector<boost::asio::const_buffer>& buffers){

	boost::unique_lock<boost::mutex> in_lock(m_in_mutex);

	switch(m_out_state){
		case state_none:

			// Output Status

			while(m_in_state <= state_none){
				m_in_cond.wait(in_lock);
			}

			buffers.push_back(status_strings::to_buffer(m_status));	
			m_out_state = state_status;

			return true;
		
		case state_status:

			// Output Header

			while(m_in_state <= state_status){
				m_in_cond.wait(in_lock);
			}

			for(std::size_t i = 0; i < m_headers.size(); ++i){
				Header& h = m_headers[i];
				buffers.push_back(boost::asio::buffer(h.name));
				buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
				buffers.push_back(boost::asio::buffer(h.value));
				buffers.push_back(boost::asio::buffer(misc_strings::crlf));
			}

			buffers.push_back(boost::asio::buffer(misc_strings::crlf));

			m_out_state = state_header;

			return true;

		case state_header:

			// Output Content

			while(m_in_state <= state_partial){
				m_in_cond.wait(in_lock);
			}

		case state_partial:

			{
				boost::unique_lock<boost::mutex> q_lock(m_q_mutex);
				while(m_q_data == false){
					m_q_cond.wait(q_lock);
				}

				std::size_t q_size = m_in.size();

				for(; q_size > 0; --q_size){
					m_out.push_back(m_in.front());
					m_in.pop();
				}
				
				m_q_data = false;

			}

			for(std::vector<std::string*>::iterator it = m_out.begin(); it != m_out.end(); ++it){
				buffers.push_back(boost::asio::buffer(*(*it)));
			}
			
			if(m_in_state == state_done){
				m_out_state = state_done;
			} else {
				m_out_state = state_partial;
			}

			return true;

		case state_done:

			return false;

	}

}

void Hydra::Reply::discard(){

	// Release buffers.

	for(std::vector<std::string*>::iterator it = m_out.begin(); it != m_out.end(); ++it){
		delete (*it);
	}

	m_out.clear();

}

