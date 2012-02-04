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
#include <vector>

#include <boost/array.hpp>

#include "exception.hpp"
#include "http.hpp"

namespace Hydra {

class Request {

public:

	Request();
	
	template < size_t T>
	bool write_buffer(boost::array<char, T>&, size_t& start, const size_t& bytes);
	
	std::string header(const std::string& key);
	void header(const std::string& key, const std::string& value);

	const std::string& version();

	const std::string& path();

	void read_buffer(std::string&);

private:

	bool parse_header(const char& c);
	bool parse_content();
	bool parse_content(const char& c);

	enum HeaderParserState {
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

	HeaderParserState m_header_state;
	bool m_header_done;

	std::string m_content;
	size_t m_content_bytes;

	std::string m_method;
	std::string m_path;
	std::string m_version;
	std::map<std::string, std::string> m_headers;
	std::vector<std::string> m_header_order;

	std::string m_buffer;

};

}

template < size_t T>
bool Hydra::Request::write_buffer(boost::array<char, T>& buffer, size_t& start, const size_t& bytes){

	for(; bytes > start; ++start){

		if(!m_header_done){

			m_header_done = parse_header(buffer[start]);

			if(m_header_done && !parse_content()){

				++start;

				return true;

			}

		} else {

			if(parse_content(buffer[start])){
			
				++start;

				return true;

			}

		}

	}

	return false;

}

#endif

