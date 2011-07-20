//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

namespace Hydra {

connection::connection(boost::asio::io_service& io_service, request_handler& handler) : m_socket(io_service), m_request_handler(handler){

}

boost::asio::ip::tcp::socket& connection::socket(){
	return m_socket;
}

void connection::start(){
	m_socket.async_read_some(boost::asio::buffer(m_buffer),
			boost::bind(&connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
}

void connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred){
	if (!e){
		boost::tribool result;
		boost::tie(result, boost::tuples::ignore) = m_request_parser.parse(
				m_request, m_buffer.data(), m_buffer.data() + bytes_transferred);

		if (result){
			m_request_handler.handle_request(m_request, m_reply);
			boost::asio::async_write(m_socket, m_reply.to_buffers(),
					boost::bind(&connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error));
		} else if (!result) {
			m_reply = reply::stock_reply(reply::bad_request);
			boost::asio::async_write(m_socket, m_reply.to_buffers(),boost::bind(&connection::handle_write, shared_from_this(), boost::asio::placeholders::error));
		}
		else
		{
			m_socket.async_read_some(boost::asio::buffer(m_buffer),
					boost::bind(&connection::handle_read, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
		}
	}

	// If an error occurs then no new asynchronous operations are started. This
	// means that all shared_ptr references to the connection object will
	// disappear and the object will be destroyed automatically after this
	// handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const boost::system::error_code& e)
{
	if (!e)
	{
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	}

	// No new asynchronous operations are started. This means that all shared_ptr
	// references to the connection object will disappear and the object will be
	// destroyed automatically after this handler returns. The connection class's
	// destructor closes the socket.
}

}
