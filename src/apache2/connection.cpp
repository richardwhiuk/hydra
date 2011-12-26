//
// apache2-client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <sstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <apache2/connection.hpp>
#include <apache2/client.hpp>

Hydra::Apache2::Connection::Connection(boost::asio::io_service& io_service, Apache2::Engine& engine, Apache2::Client& client) : m_resolver(io_service), m_socket(io_service), m_engine(engine), m_client(client), m_req_data(false){

}

void Hydra::Apache2::Connection::init(){

	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.
	boost::asio::ip::tcp::resolver::query query(m_client.server(), m_client.port());
	m_resolver.async_resolve(query,
		boost::bind(&Hydra::Apache2::Connection::handle_resolve, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));

}

void Hydra::Apache2::Connection::run(Hydra::Connection::Ptr con){
	m_connection = con;

	std::ostream req_stream(&m_request);
	Request& req = con->request();
	req_stream << req.method << " " << req.uri << " HTTP/1.0" << "\r\n";
	for(std::vector<Header>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it){
		if(it->name != "Connection"){
			req_stream << it->name << ": " << it->value << "\r\n";
		}
		
	}

	req_stream << "\r\n";

	// Notify that the request data is available.

	{
		boost::lock_guard<boost::mutex> req_lock(m_req_mutex);
		m_req_data = true;
	}
	
	m_req_cond.notify_one();

}

Hydra::Apache2::Connection::~Connection(){

}

void Hydra::Apache2::Connection::handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
	if (!err){
		// Attempt a connection to the first endpoint in the list. Each endpoint
		// will be tried until we successfully establish a connection.
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
			boost::bind(&Hydra::Apache2::Connection::handle_connect, shared_from_this(),
				boost::asio::placeholders::error, ++endpoint_iterator));
	} else {
		std::cerr << "Hydra: Apache2: Resolve: " << err.message() << std::endl;
	}
}

void Hydra::Apache2::Connection::handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator){ 
	if (!err){

		// Need to block until we have the request.

		perform_request();

	} else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()){
		// The connection failed. Try the next endpoint in the list.
		m_socket.close();
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
				boost::bind(&Hydra::Apache2::Connection::handle_connect, shared_from_this(),
					boost::asio::placeholders::error, ++endpoint_iterator));
	} else {
		std::cerr << "Hydra: Apache2: Connect: " << err.message() << "\n";
	}
}

void Hydra::Apache2::Connection::perform_request(){

	boost::unique_lock<boost::mutex> req_lock(m_req_mutex);

	while(!m_req_data){
		m_req_cond.wait(req_lock);
	}


	// The connection was successful and we have the data. Perform the request
	boost::asio::async_write(m_socket, m_request,
			boost::bind(&Hydra::Apache2::Connection::handle_write_request, shared_from_this(),
				boost::asio::placeholders::error));
	
}

void Hydra::Apache2::Connection::handle_write_request(const boost::system::error_code& err){
	if (!err){
		// Read the response status line.
		boost::asio::async_read_until(m_socket, m_response, "\r\n",
				boost::bind(&Hydra::Apache2::Connection::handle_read_status_line, shared_from_this(),
					boost::asio::placeholders::error));
	} else {
		std::cerr << "Hydra: Apache2: Write: " << err.message() << "\n";
	}
}

void Hydra::Apache2::Connection::handle_read_status_line(const boost::system::error_code& err){
	if (!err){
		// Check that response is OK.
		std::istream m_responsestream(&m_response);
		std::string http_version;
		m_responsestream >> http_version;
		unsigned int status_code;
		m_responsestream >> status_code;
		std::string status_message;
		std::getline(m_responsestream, status_message);

		if (!m_responsestream || http_version.substr(0, 5) != "HTTP/"){
			std::cerr << "Hydra: Apache2: Invalid response" << std::endl;
			return;
		}

		m_connection->reply().status(Hydra::Reply::status_type(status_code));

		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n",
				boost::bind(&Hydra::Apache2::Connection::handle_read_headers, shared_from_this(),
					boost::asio::placeholders::error));
	} else {
		std::cerr << "Hydra: Apache2: Read Status Error: " << err << std::endl;
	}
}

void Hydra::Apache2::Connection::handle_read_headers(const boost::system::error_code& err){
	if (!err){
		// Process the response headers.
		std::istream response_stream(&m_response);
		std::string hstr;

		while (std::getline(response_stream, hstr, '\n') && hstr != "\r"){
			size_t a;
			a = hstr.find(':');		
			Hydra::Header nh;
			nh.name = hstr.substr(0, a);
			nh.value = hstr.substr(a+2,hstr.length() - a - 3); 
			if(nh.name != "Connection" && nh.name != "Keep-Alive"){
				m_connection->reply().header(nh);
			}
		}

		m_connection->reply().headers_complete();

		// Write whatever content we already have to output.
		if (m_response.size() > 0){
			std::stringstream ss;
			std::copy(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
			m_connection->reply().content(ss.str());
		} else {
			m_connection->reply().content();
		}

		m_connection->reply().content_bind(boost::bind(&Hydra::Apache2::Connection::perform_read, shared_from_this()));

	} else {
		std::cerr << "Hydra: Apache2: Read Header Error: " << err << "\n";
	}
}

void Hydra::Apache2::Connection::perform_read(){

	boost::asio::async_read(m_socket, boost::asio::buffer(m_response_buffer),
			boost::asio::transfer_at_least(1),
			boost::bind(&Hydra::Apache2::Connection::handle_read_content, shared_from_this(),
				boost::asio::placeholders::error));

}

void Hydra::Apache2::Connection::handle_read_content(const boost::system::error_code& err){
	
	if (!err){
		m_response.commit(1024);

		// Write all of the data that has been read so far.
		if (m_response.size() > 0){
			std::stringstream ss;
			std::copy(m_response_buffer.begin(), m_response_buffer.end(), std::ostreambuf_iterator<char>(ss));
			m_connection->reply().content(ss.str());
		} else {
			perform_read();
		}

	} else if (err != boost::asio::error::eof){
		std::cerr << "Hydra: Apache2: Read Content Error: " << err << "\n";
	} else {
		// Done

		m_connection->reply().finish();
		m_connection.reset();

	}

}

