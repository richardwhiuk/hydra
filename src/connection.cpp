//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <iostream>
#include <boost/bind.hpp>
#include "request_handler.hpp"

namespace Hydra {

Connection::Connection(boost::asio::io_service& io_service, Request_Handler& handler) : m_socket(io_service), m_request_handler(handler), m_writing(false){

}

Request& Connection::request(){
	return m_request;
}

Reply& Connection::reply(){
	return m_reply;
}

boost::asio::ip::tcp::socket& Connection::socket(){
	return m_socket;
}

void Connection::start(){
	m_socket.async_read_some(boost::asio::buffer(m_buffer),
			boost::bind(&Connection::read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
}

void Connection::read(const boost::system::error_code& e, std::size_t bytes_transferred){
	if (!e){

		boost::tribool result;
		boost::tie(result, boost::tuples::ignore) = m_request_parser.parse(*this, m_buffer.data(), m_buffer.data() + bytes_transferred);

		if (result){	
	
			m_request_handler.handle_request(*this);

			perform_finish();
		} else if (!result) {
			m_reply = Reply::Stock(Reply::bad_request);

			perform_finish();
		}
		else
		{
			m_socket.async_read_some(
				boost::asio::buffer(m_buffer),
				boost::bind(
					&Connection::read, 
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			);
		}
	}

	// If an error occurs then no new asynchronous operations are started. This
	// means that all shared_ptr references to the Connection object will
	// disappear and the object will be destroyed automatically after this
	// handler returns. The Connection class's destructor closes the socket.
}

void Connection::perform_write(){

	std::cout << "Attempting to Write using socket " << &m_socket << " and connection " << this << std::endl;

	boost::unique_lock<boost::mutex> lock(m_write_mux);
	while(m_writing){
		std::cout << "." << std::endl;
		m_write_cv.wait(lock);
	}


	std::cout << "Writing." << std::endl;

	m_writing = true;

	boost::asio::async_write(
		m_socket, 
		m_reply.buffers(),
		boost::bind(
			&Connection::write, 
			shared_from_this(),
			boost::asio::placeholders::error
		)
	);
}

void Connection::perform_finish(){

	std::cout << "Attempting to Finish using socket " << &m_socket << " and connection " << this << std::endl;

	boost::unique_lock<boost::mutex> lock(m_write_mux);
	while(m_writing){
		m_write_cv.wait(lock);
	}
	
	m_writing = true;

	std::cout << "Finishing........." << std::endl;
	
	boost::asio::async_write(
		m_socket, 
		m_reply.buffers(),
		boost::bind(
			&Connection::finish, 
			shared_from_this(),
			boost::asio::placeholders::error
		)
	);
}

void Connection::write(const boost::system::error_code& e){
	
	std::cout << "Discarding...." << std::endl;

	m_reply.discard();

	std::cout << "Attempting to stop writing..." << std::endl;

	{
		boost::lock_guard<boost::mutex> lock(m_write_mux);
		m_writing = false;
	}

	std::cout << "Stopped Writing..........." << std::endl;

	m_write_cv.notify_one();
}

void Connection::finish(const boost::system::error_code& e){

	m_reply.discard();

	{
		boost::lock_guard<boost::mutex> lock(m_write_mux);
		m_writing = false;
	}

	std::cout << "Stopped Finishing..........." << std::endl;

	m_write_cv.notify_one();

	if (!e){
		// Initiate graceful Connection closure.
		boost::system::error_code ignored_ec;

		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	}

	// No new asynchronous operations are started. This means that all shared_ptr
	// references to the Connection object will disappear and the object will be
	// destroyed automatically after this handler returns. The Connection class's
	// destructor closes the socket.
}

}

