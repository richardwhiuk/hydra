//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CONNECTION_HPP
#define HYDRA_FILE_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <hydra.hpp>
#include <reply.hpp>
#include <request.hpp>
#include <request_parser.hpp>

namespace Hydra {

/// Represents a single connection from a client.

// Handles IO between client and server via async messages.

class Connection : public boost::enable_shared_from_this<Connection>, private boost::noncopyable {

public:

	/// Construct a connection with the given io_service.
	explicit Connection(boost::asio::io_service& io_service, Request_Handler& handler);

	/// Get the socket associated with the connection.
	boost::asio::ip::tcp::socket& socket();

	/// Start the first asynchronous operation for the connection.
	void start();

	Request& request();
	
	Reply& reply();

	// Called by handler to send buffered data;

	void perform_write();

	// Called to terminate request

	void perform_finish();

	typedef boost::shared_ptr<Connection> Ptr;


private:

	void read(const boost::system::error_code& e, std::size_t bytes_transferred);


	// Called by handler to send buffered data.

	void write(const boost::system::error_code& e);

	// Called by connection to send remaining data.

	void finish(const boost::system::error_code& e);

	/// Socket for the connection.
	boost::asio::ip::tcp::socket m_socket;

	/// The handler used to process the incoming request.
	Request_Handler& m_request_handler;

	/// Buffer for incoming data.
	boost::array<char, 8192> m_buffer;

	/// The incoming request.
	Request m_request;

	/// The parser for the incoming request.
	Request_Parser m_request_parser;

	/// The reply to be sent back to the client.
	Reply m_reply;

	/// Prevent write overlap.
	boost::mutex m_write_mux;
	boost::condition_variable m_write_cv;
	bool m_writing;


};

}

#endif // HTTP_SERVER2_CONNECTION_HPP
