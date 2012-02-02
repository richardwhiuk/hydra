//
// response.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "response.hpp"

#include <sstream>

Hydra::Response::Response() : m_read_state(NONE), m_write_state(NONE), m_parse_state(PROTOCOL_H){

}

std::string Hydra::Response::code_text(){

	switch(m_code){

		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";
		case 103: return "Checkpoint";
		case 122: return "Request-URI too long";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 207: return "Multi-Status";
		case 208: return "Already Reported";
		case 226: return "IM Used";
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 306: return "Switch Proxy";
		case 307: return "Temporary Redirect";
		case 308: return "Resume Incomplete";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Request Entity Too Large";
		case 414: return "Request-URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Requested Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";
		case 422: return "Unprocessable Entity";
		case 423: return "Locked";
		case 424: return "Failed Dependency";
		case 425: return "Unordered Collection";
		case 426: return "Upgrade Required";
		case 428: return "Precondition Required";
		case 429: return "Too Many Requests";
		case 431: return "Request Header Fields Too Large";
		case 444: return "No Response";
		case 449: return "Retry With";
		case 450: return "Blocked by Windows Parental Controls";
		case 499: return "Client Closed Request";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		case 506: return "Variant Also Negotiates";
		case 507: return "Insufficient Storage";
		case 508: return "Loop Detected";
		case 509: return "Bandwidth Limit Exceeded";
		case 510: return "Not Extended";
		case 511: return "Network Authentication Required";
		case 598: return "Network read timeout error";
		case 599: return "Network connect timeout error";

	}

	return "Internal Server Error";

}

void Hydra::Response::error(int code){

	m_version = "1.0";

	m_code = code;

	m_headers["Content-Type"] = "text/html";
	m_headers["Server"] = "Hydra";

	std::stringstream buffer("<html><body><h1>");

	buffer << code << " " << code_text() << "</h1></body></html>";

	std::stringstream ss;

	ss << buffer.str().length();
	
	m_headers["Content-Length"] = ss.str();

	m_content.push_back(buffer.str());

	m_read_state = DONE;

	if(m_write){
	
		m_write();

	}

}

void Hydra::Response::read_buffer(std::string& buffer){

	std::stringstream response("");

	switch(m_write_state){
		case NONE:
			if(m_read_state == NONE){
				break;
			}

			response << "HTTP/" << m_version << " " << m_code << code_text() << "\r\n";

			for(std::map<std::string, std::string>::iterator it = m_headers.begin(); it != m_headers.end(); ++it){

				response << it->first << ": " << it->second << "\r\n";

			}

			response << "\r\n";

		case CONTENT:

			for(size_t i = m_content.size(); i > 0; --i){

				response << m_content.front();
				m_content.pop_front();

			}
	
			if(m_read_state == DONE){

				m_write_state = DONE;

			} else {

				m_write_state = CONTENT;

			}

		case DONE:

			break;
	}

	buffer = response.str();

}

void Hydra::Response::bind_read(boost::function<void()> reader){
	
	m_read = reader;

}

void Hydra::Response::bind_write(boost::function<void()> writer){

	m_write = writer;

}

void Hydra::Response::bind_finish(boost::function<void()> finish){

	m_finish = finish;

}

void Hydra::Response::read(){

	if(m_read_state == DONE && m_write_state == DONE){

		// Everything is done.

		m_read = NULL;
		m_write = NULL;

		if(m_finish){
			m_finish();
		}

		return;

	} else if(m_read_state != m_write_state) {

		// There is data to be written

		if(m_write){
			m_write();
		}

		return;

	} else if(m_read_state == CONTENT && m_content.size() > 0){

		// There is content data available.

		if(m_write){
			m_write();
		}

		return;

	} else {

		// All available data has been written - go read some more.

		if(m_read){
			m_read();
		}

		return;

	}

}

void Hydra::Response::write(){

	m_write();

}

void Hydra::Response::done(){

	m_read_state = DONE;

	m_read = NULL;

	m_finish();

}

