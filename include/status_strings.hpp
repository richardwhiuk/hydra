//
// status_strings.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "reply.hpp"
#include <string>
#include <boost/lexical_cast.hpp>

namespace Hydra {

namespace status_strings {

	const std::string ok = "HTTP/1.0 200 OK\r\n";
	const std::string created = "HTTP/1.0 201 Created\r\n";
	const std::string accepted = "HTTP/1.0 202 Accepted\r\n";
	const std::string no_content = "HTTP/1.0 204 No Content\r\n";
	const std::string multiple_choices = "HTTP/1.0 300 Multiple Choices\r\n";
	const std::string moved_permanently = "HTTP/1.0 301 Moved Permanently\r\n";
	const std::string moved_temporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
	const std::string not_modified = "HTTP/1.0 304 Not Modified\r\n";
	const std::string bad_request = "HTTP/1.0 400 Bad Request\r\n";
	const std::string unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
	const std::string forbidden = "HTTP/1.0 403 Forbidden\r\n";
	const std::string not_found = "HTTP/1.0 404 Not Found\r\n";
	const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
	const std::string not_implemented = "HTTP/1.0 501 Not Implemented\r\n";
	const std::string bad_gateway = "HTTP/1.0 502 Bad Gateway\r\n";
	const std::string service_unavailable = "HTTP/1.0 503 Service Unavailable\r\n";

	boost::asio::const_buffer to_buffer(Reply::status_type status);

}

}

