//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_REQUEST_HPP
#define HYDRA_FILE_REQUEST_HPP

#include <string>
#include <map>

#include <boost/array.hpp>

#include "exception.hpp"
#include "http.hpp"

namespace Hydra {

class Request {

public:

	Request();
	
	template < size_t T>
	bool write_buffer(boost::array<char, T>&, size_t bytes);
	
	std::string header(const std::string& key);
	void header(const std::string& key, const std::string& value);

	const std::string& path();

	void read_buffer(std::string&);

private:

	enum ParserState {
		START,
		METHOD_CHAR,
		PATH_START_CHAR,
		PATH_CHAR,
		VERSION_H,
		VERSION_HT,
		VERSION_HTT,
		VERSION_HTTP,
		VERSION_SLASH,
		VERSION_NUMBER,
		HEADER_KEY,
		HEADER_KEY_CHAR,
		HEADER_SPACE,
		HEADER_VALUE,
		HEADER_VALUE_CHAR,
		CRLF,
		FINAL_CRLF,
		DONE
	};

	ParserState m_state;

	std::string m_method;
	std::string m_path;
	std::string m_version;
	std::map<std::string, std::string> m_headers;

	std::string m_buffer;

};

}

template < size_t T>
bool Hydra::Request::write_buffer(boost::array<char, T>& buffer, size_t bytes){

	using namespace Hydra::HTTP;

	for(size_t i = 0; (i < bytes) && (m_state != DONE); ++i){

		switch(m_state){

			case START:

				if(is_alpha(buffer[i])){
					m_method.push_back(buffer[i]);
					m_state = METHOD_CHAR;
				} else {
					throw new Exception("Invalid Method in HTTP Request");
				}

				break;
	
			case METHOD_CHAR:

				if(is_alpha(buffer[i])){
					m_method.push_back(buffer[i]);
				} else if(buffer[i] == ' ') {
					m_state = PATH_CHAR;
				} else {
					throw new Exception("Invalid Method in HTTP Request");
				}

				break;

			case PATH_START_CHAR:

				if(!is_ctl(buffer[i]) && is_char(buffer[i])){
					m_path = buffer[i];
					m_state = PATH_CHAR;
				} else {
					throw new Exception("Invalid Path in HTTP Request");
				}

				break;

			case PATH_CHAR:

				if(buffer[i] == ' '){
					m_state = VERSION_H;
				} else if(!is_ctl(buffer[i]) && is_char(buffer[i])){
					m_path.push_back(buffer[i]);
				} else {
					throw new Exception("Invalid Path in HTTP Request");
				}

				break;

			case VERSION_H:

				if(buffer[i] == 'H'){
					m_state = VERSION_HT;
				} else {
					throw new Exception("Invalid Protocol in HTTP Request");
				} 

				break;

			case VERSION_HT:

				if(buffer[i] == 'T'){
					m_state = VERSION_HTT;
				} else {
					throw new Exception("Invalid Protocol in HTTP Request");
				}

				break;
			
			case VERSION_HTT:

				if(buffer[i] == 'T'){
					m_state = VERSION_HTTP;
				} else {
					throw new Exception("Invalid Protocol in HTTP Request");
				}

				break;
			
			case VERSION_HTTP:

				if(buffer[i] == 'P'){
					m_state = VERSION_SLASH;
				} else {
					throw new Exception("Invalid Protocol in HTTP Request");
				}

				break;
			
			case VERSION_SLASH:

				if(buffer[i] == '/'){
					m_state = VERSION_NUMBER;
				} else {
					throw new Exception("Invalid Protocol in HTTP Request");
				}

				break;
			
			case VERSION_NUMBER:

				if(is_digit(buffer[i]) || buffer[i] == '.'){
					m_version.push_back(buffer[i]);
				} else if(buffer[i] == '\r') {
					m_state = CRLF;
				}

				break;

			case CRLF:

				if(buffer[i] == '\n'){
					m_state = HEADER_KEY;
				} else {
					throw new Exception("Invalid syntax in HTTP Request");
				}

				break;

			case HEADER_KEY:

				if(buffer[i] == '\r'){
					m_state = FINAL_CRLF;
				} else if(is_char(buffer[i]) && !is_ctl(buffer[i]) && !is_special(buffer[i])){
					m_buffer = buffer[i];
					m_state = HEADER_KEY_CHAR;
				} else {
					throw new Exception("Invalid header in HTTP Request");
				}

				break;

			case HEADER_KEY_CHAR:

				if(is_char(buffer[i]) && !is_ctl(buffer[i]) && !is_special(buffer[i])){
					m_buffer.push_back(buffer[i]);
				} else if(buffer[i] == ':'){
					m_state = HEADER_SPACE;
				} else {
					throw new Exception("Invalid header in HTTP Request");
				}

				break;

			case HEADER_SPACE:

				if(buffer[i] == ' '){
					m_state = HEADER_VALUE;
				} else { 
					throw new Exception("Invalid header in HTTP Request");
				}

				break;

			case HEADER_VALUE:

				if(is_ctl(buffer[i])){
					throw new Exception("Invalid header in HTTP Request");
				} else {
					m_headers[m_buffer] = buffer[i];
					m_state = HEADER_VALUE_CHAR;
				}

				break;

			case HEADER_VALUE_CHAR:

				if(buffer[i] == '\r'){
					m_state = CRLF;
				} else if(!is_ctl(buffer[i])){
					m_headers[m_buffer].push_back(buffer[i]);
				} else {
					throw new Exception("Invalid header in HTTP Request");
				}

				break;

			case FINAL_CRLF:

				if(buffer[i] == '\n'){
					m_state = DONE;
				} else {
					throw new Exception("Invalid termination in HTTP Request");
				}

				break;	

			case DONE:

				break;

		}

	}

	if(m_state == DONE){
		return true;
	} else {
		return false;
	}

}

#endif

