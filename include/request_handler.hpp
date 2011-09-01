//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_REQUEST_HANDLER_HPP
#define HYDRA_FILE_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <connection.hpp>

namespace Hydra {

class Server;

/// The common handler for all incoming requests.
class Request_Handler : private boost::noncopyable
{
public:
	/// Construct with a directory containing files to be served.
	Request_Handler(Server* server);

	/// Handle a request and produce a reply.
	void handle_request(Hydra::Connection::Ptr con);

private:

	Server* m_server;

};

}

#endif

