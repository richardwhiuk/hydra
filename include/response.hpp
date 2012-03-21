//
// response.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>
#include <map>
#include <list>

#include <boost/function.hpp>
#include <boost/asio.hpp>

#include "http.hpp"
#include "exception.hpp"

namespace Hydra {

class Response {

public:

	Response();
	~Response();

	int code();

	void header(const std::string& key, const std::string& value);

	void error(int code);

	void read_buffer(std::string&);

	template < size_t T>
	void write_buffer(boost::array<char, T>&, size_t bytes);

	void read();
	void write();

	void done();

	void bind_read(boost::function<void()>);
	void bind_write(boost::function<void()>);
	void bind_finish(boost::function<void()>);

private:

	enum State {
		NONE,
		CONTENT,
		DONE
	};

	enum ParserState {
		PROTOCOL_H,
		PROTOCOL_HT,
		PROTOCOL_HTT,
		PROTOCOL_HTTP,
		PROTOCOL_SLASH,
		VERSION_START,
		VERSION,
		CODE_START,
		CODE,
		CODE_DESC,
		CRLF,
		HEADER,
		HEADER_KEY,
		HEADER_VALUE_SPACE,
		HEADER_VALUE,
		FINAL_CRLF,
		BODY
	};

	boost::function<void()> m_read;
	boost::function<void()> m_write;
	boost::function<void()> m_finish;

	std::string code_text();

	State m_read_state;
	State m_write_state;

	ParserState m_parse_state;

	std::string m_parse_buffer;

	std::string m_version;
	int m_code;
	std::map<std::string, std::string> m_headers;

	std::list<std::string> m_content;


};

}

template < size_t T>
void Hydra::Response::write_buffer(boost::array<char, T>& buffer, size_t bytes){

	using namespace Hydra::HTTP;

	size_t i = 0;

	for(; i < bytes && m_parse_state != BODY; ++i){

		switch(m_parse_state){

			case PROTOCOL_H:
				if(buffer[i] != 'H'){
					throw new Exception("Hydra->Response->Invalid Protocol");
				}
				m_parse_state = PROTOCOL_HT;
				break;

			case PROTOCOL_HT:
				if(buffer[i] != 'T'){
					throw new Exception("Hydra->Response->Invalid Protocol");
				}
				m_parse_state = PROTOCOL_HTT;
				break;

			case PROTOCOL_HTT:
				if(buffer[i] != 'T'){
					throw new Exception("Hydra->Response->Invalid Protocol");
				}
				m_parse_state = PROTOCOL_HTTP;
				break;

			case PROTOCOL_HTTP:
				if(buffer[i] != 'P'){
					throw new Exception("Hydra->Response->Invalid Protocol");
				}
				m_parse_state = PROTOCOL_SLASH;
				break;
			case PROTOCOL_SLASH:
				if(buffer[i] != '/'){
					throw new Exception("Hydra->Response->Invalid Protocol");
				}
				m_parse_state = VERSION_START;
				break;
			case VERSION_START:
				if(is_digit(buffer[i]) || buffer[i] == '.'){
					m_version = buffer[i];
					m_parse_state = VERSION;
				} else {
					throw new Exception("Hydra->Response->Invalid Version");
				}
				break;
			case VERSION:
				if(is_digit(buffer[i]) || buffer[i] == '.'){
					m_version.push_back(buffer[i]);
				} else if(buffer[i] == ' '){
					m_parse_state = CODE_START;
				} else {
					throw new Exception("Hydra->Response->Invalid Version");
				}
				break;
			case CODE_START:
				if(is_digit(buffer[i])){
					m_code = (buffer[i] - '0');
					m_parse_state = CODE;
				} else {
					throw new Exception("Hydra->Response->Invalid Code");
				}
				break;
			case CODE:
				if(is_digit(buffer[i])){
					m_code = (m_code * 10) + (buffer[i] - '0');
				} else if(buffer[i] == ' '){
					m_parse_state = CODE_DESC;
				} else {
					throw new Exception("Hydra->Response->Invalid Code");
				}
				break;
			case CODE_DESC:
				if(is_alpha(buffer[i]) || buffer[i] == ' '){
					
				} else if(buffer[i] == '\r'){
					m_parse_state = CRLF;
				} else {
					throw new Exception("Hydra->Response->Invalid Code Description");
				}
				break;
			case CRLF:
				if(buffer[i] == '\n'){
					m_parse_state = HEADER;
				} else {
					throw new Exception("Hydra->Response->Invalid Line Terminator");
				}
				break;
			case HEADER:
				if(buffer[i] == '\r'){
					m_parse_state = FINAL_CRLF;
				} else if(is_alpha(buffer[i])){
					m_parse_buffer = buffer[i];
					m_parse_state = HEADER_KEY;
				} else {
					throw new Exception("Hydra->Response->Invalid Header");
				}
				break;
			case HEADER_KEY:
				if(is_char(buffer[i]) && !is_ctl(buffer[i]) && !is_special(buffer[i])){
					m_parse_buffer.push_back(buffer[i]);
				} else if(buffer[i] == ':'){
					m_parse_state = HEADER_VALUE_SPACE;
					m_headers[m_parse_buffer] = "";
				} else {
					throw new Exception("Hydra->Response->Invalid Header");
				}
				break;
			case HEADER_VALUE_SPACE:
				if(buffer[i] == ' '){
					
				} else if(buffer[i] == '\r'){
					m_parse_state = CRLF;
				} else if(!is_ctl(buffer[i])){
					m_headers[m_parse_buffer].push_back(buffer[i]);
					m_parse_state = HEADER_VALUE;
				} else {
					throw new Exception("Hydra->Response->Invalid Header");
				}
				break;
			case HEADER_VALUE:
				if(buffer[i] == '\r'){
					m_parse_state = CRLF;
				} else if(!is_ctl(buffer[i])){
					m_headers[m_parse_buffer].push_back(buffer[i]);
				} else {
					throw new Exception("Hydra->Response->Invalid Header");
				}
				break;
			case FINAL_CRLF:
				if(buffer[i] == '\n'){
					m_parse_state = BODY;
					break;
				}
			default:
				throw new Exception("Hydra->Response->Invalid Header");
				
		}


	}

	if(m_parse_state == BODY && i < bytes){
		std::string data(buffer.begin() + i, buffer.begin() + bytes);

		m_content.push_back(data);

		m_read_state = CONTENT;
	}

}

