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

Hydra::Request::Request() : m_header_state(START), m_header_done(false), m_content(""){

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
				m_header_state = HEADER_SPACE;
			} else {
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case HEADER_SPACE:

			if(in == ' '){
				m_header_state = HEADER_VALUE;
			} else { 
				throw new Exception("Invalid header in HTTP Request");
			}

			break;

		case HEADER_VALUE:

			if(is_ctl(in)){
				throw new Exception("Invalid header in HTTP Request");
			} else {
				m_headers[m_buffer] = in;
				m_header_state = HEADER_VALUE_CHAR;
			}

			break;

		case HEADER_VALUE_CHAR:

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
	}


	return false;

}

bool Hydra::Request::parse_content(const char& in){

	m_content.push_back(in);

	m_content_bytes --;

	return (m_content_bytes == 0);

}

