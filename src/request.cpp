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

Hydra::Request::Request() : m_header_state(START), m_header_done(false), m_content(""), m_transfer_chunked(CHUNKED_NONE){

}

const std::string& Hydra::Request::path(){
	return m_path;
}

const std::string& Hydra::Request::version(){
	return m_version;
}

const std::string& Hydra::Request::method(){
	return m_method;
}

void Hydra::Request::header(const std::string& key, const std::string& value){

	if(m_headers.find(key) == m_headers.end()){
		m_header_order.push_back(key);
	}

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

	for(std::vector<std::string>::iterator it = m_header_order.begin(); it != m_header_order.end(); ++it){

		ss << *it << ": " << m_headers[*it] << "\r\n";

	}

	ss << "\r\n";

	ss << m_content;

	buffer = ss.str();

}

bool Hydra::Request::parse_header(const char& in){

	using namespace Hydra::HTTP;

	switch(m_header_state){

		case START:

			if(is_alpha(in)){
				m_method.push_back(in);
				m_header_state = METHOD_CHAR;
			} else {
				throw new Exception("Invalid Method in HTTP Request");
			}

			break;
	
		case METHOD_CHAR:

			if(is_alpha(in)){
				m_method.push_back(in);
			} else if(in == ' ') {
				m_header_state = PATH_CHAR;
			} else {
				throw new Exception("Invalid Method in HTTP Request");
			}

			break;

		case PATH_START_CHAR:

			if(!is_ctl(in) && is_char(in)){
				m_path = in;
				m_header_state = PATH_CHAR;
			} else {
				throw new Exception("Invalid Path in HTTP Request");
			}

			break;

		case PATH_CHAR:

			if(in == ' '){
				m_header_state = VERSION_H;
			} else if(!is_ctl(in) && is_char(in)){
				m_path.push_back(in);
			} else {
				throw new Exception("Invalid Path in HTTP Request");
			}

			break;

		case VERSION_H:

			if(in == 'H'){
				m_header_state = VERSION_HT;
			} else {
				throw new Exception("Invalid Protocol in HTTP Request");
			} 

			break;

		case VERSION_HT:

			if(in == 'T'){
				m_header_state = VERSION_HTT;
			} else {
				throw new Exception("Invalid Protocol in HTTP Request");
			}

			break;
			
		case VERSION_HTT:

			if(in == 'T'){
				m_header_state = VERSION_HTTP;
			} else {
				throw new Exception("Invalid Protocol in HTTP Request");
			}

			break;
			
		case VERSION_HTTP:

			if(in == 'P'){
				m_header_state = VERSION_SLASH;
			} else {
				throw new Exception("Invalid Protocol in HTTP Request");
			}

			break;
			
		case VERSION_SLASH:

			if(in == '/'){
				m_header_state = VERSION_NUMBER;
			} else {
				throw new Exception("Invalid Protocol in HTTP Request");
			}

			break;
			
		case VERSION_NUMBER:

			if(is_digit(in) || in == '.'){
				m_version.push_back(in);
			} else if(in == '\r') {
				m_header_state = CRLF;
			}

			break;

		case CRLF:

			if(in == '\n'){
				m_header_state = HEADER_KEY;
			} else {
				throw new Exception("Invalid syntax in HTTP Request");
			}

			break;

		case HEADER_KEY:

			if(in == '\r'){
				m_header_state = FINAL_CRLF;
			} else if(is_char(in) && !is_ctl(in) && !is_special(in)){
				m_buffer = in;
				m_header_state = HEADER_KEY_CHAR;
			} else {
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case HEADER_KEY_CHAR:

			if(is_char(in) && !is_ctl(in) && !is_special(in)){
				m_buffer.push_back(in);
			} else if(in == ':'){
				m_header_state = HEADER_VALUE_SPACE;
				m_headers[m_buffer] = "";
				m_header_order.push_back(m_buffer);
			} else {
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case HEADER_VALUE_SPACE:

			if(in == ' '){
				
			} else if(in == '\r'){
				m_header_state = CRLF;
			} else if(!is_ctl(in)){
				m_headers[m_buffer].push_back(in);
				m_header_state = HEADER_VALUE;
			} else {
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case HEADER_VALUE:

			if(in == '\r'){
				m_header_state = CRLF;
			} else if(!is_ctl(in)){
				m_headers[m_buffer].push_back(in);
			} else {
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case FINAL_CRLF:

			if(in == '\n'){
				m_header_state = DONE;
			} else {
				throw new Exception("Invalid termination in HTTP Request");
			}

			break;	

		case DONE:

			break;

	}

	return (m_header_state == DONE);

}

bool Hydra::Request::parse_content(){

	if(m_headers.find("Content-Length") != m_headers.end()){

		std::istringstream( m_headers["Content-Length"] ) >> m_content_bytes;

		return true;

	} else if(m_headers.find("Transfer-Encoding") != m_headers.end() && m_headers["Transfer-Encoding"] == "chunked"){

		m_content_bytes = 0;

		m_transfer_chunked = CHUNKED_HEADER;
		
		return true;

	}


	return false;

}

bool Hydra::Request::parse_content(const char& in){

	switch(m_transfer_chunked){

		case CHUNKED_NONE:

			m_content.push_back(in);

			m_content_bytes --;

			return (m_content_bytes == 0);

		case CHUNKED_HEADER:

			if(in == '\r'){

				m_transfer_chunked = CHUNKED_HEADER_CRLF;

			} else if(in >= '0' && in <= '9'){

				m_content_bytes = (m_content_bytes * 16) + (in - '0');

			} else if(in >= 'A' && in <= 'F'){

				m_content_bytes = (m_content_bytes * 16) + (in - 'A' + 10);

			} else if(in >= 'a' && in <= 'f'){

				m_content_bytes = (m_content_bytes * 16) + (in - 'a' + 10);

			} else {
	
				throw new Exception("Unknown chunk header");
	
			}

			return false;

		case CHUNKED_HEADER_CRLF:

			if(in == '\n'){

				if(m_content_bytes == 0){
					return true;
				} else {
					m_transfer_chunked = CHUNKED_DATA;
				}

			} else {

				throw new Exception("Unknown chunk header - expected new line.");

			}

			return false;

		case CHUNKED_DATA:

			m_content_bytes --;

			m_content.push_back(in);

			if(m_content_bytes == 0){

				m_transfer_chunked = CHUNKED_DATA_CR;

			}

			return false;

		case CHUNKED_DATA_CR:

			if(in == '\r'){

				m_transfer_chunked = CHUNKED_DATA_CRLF;

			} else {

				throw new Exception("Unknown chunk data terminator - expected carriage return");

			}

			return false;

		case CHUNKED_DATA_CRLF:

			if(in == '\n'){

				m_transfer_chunked = CHUNKED_HEADER;

			} else {

				throw new Exception("Unknown chunk data terminator - expected carriage return");

			}

			return false;

	}

	return false;
}

