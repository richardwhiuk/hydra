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
#include "apache2.hpp"

Hydra::Apache2::Client::Connection::Connection(boost::asio::io_service& io_service, Hydra::Connection& con, Apache2::Client& client) : m_resolver(io_service), m_socket(io_service), m_connection(con), m_client(client){
	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	std::ostream m_requeststream(&m_request);
	Request& req = con.request();
	m_requeststream << req.method << " " << req.uri << " HTTP/1.0" << "\r\n";
	for(std::vector<Header>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it){
		if(it->name != "Connection"){
			m_requeststream << it->name << ": " << it->value << "\r\n";
		}
		
	}
	m_requeststream << "\r\n";

	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.
	boost::asio::ip::tcp::resolver::query query(m_client.server(), m_client.port());
	m_resolver.async_resolve(query,
		boost::bind(&Hydra::Apache2::Client::Connection::handle_resolve, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
}

Hydra::Apache2::Client::Connection::~Connection(){

}

void Hydra::Apache2::Client::Connection::handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
	if (!err){
		// Attempt a connection to the first endpoint in the list. Each endpoint
		// will be tried until we successfully establish a connection.
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
				boost::bind(&Hydra::Apache2::Client::Connection::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
	} else {
		std::cerr << "Hydra: Apache2: " << err.message() << std::endl;
	}
}

void Hydra::Apache2::Client::Connection::handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator){ 
	if (!err){
		// The connection was successful. Send the request.
		boost::asio::async_write(m_socket, m_request,
				boost::bind(&Hydra::Apache2::Client::Connection::handle_write_request, this,
					boost::asio::placeholders::error));
	} else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()){
		// The connection failed. Try the next endpoint in the list.
		m_socket.close();
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
				boost::bind(&Hydra::Apache2::Client::Connection::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
	} else {
		std::cerr << "Hydra: Apache2: " << err.message() << "\n";
	}
}

void Hydra::Apache2::Client::Connection::handle_write_request(const boost::system::error_code& err){
	if (!err){
		// Read the response status line.
		boost::asio::async_read_until(m_socket, m_response, "\r\n",
				boost::bind(&Hydra::Apache2::Client::Connection::handle_read_status_line, this,
					boost::asio::placeholders::error));
	} else {
		std::cout << "Hydra: Apache2: " << err.message() << "\n";
	}
}

void Hydra::Apache2::Client::Connection::handle_read_status_line(const boost::system::error_code& err){
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
			std::cout << "Hydra: Apache2: Invalid response" << std::endl;
			return;
		}

		m_connection.reply().status = Hydra::Reply::status_type(status_code);

		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n",
				boost::bind(&Hydra::Apache2::Client::Connection::handle_read_headers, this,
					boost::asio::placeholders::error));
	} else {
		std::cout << "Hydra: Apache2: Error: " << err << std::endl;
	}
}

void Hydra::Apache2::Client::Connection::handle_read_headers(const boost::system::error_code& err){
	if (!err){
		// Process the response headers.
		std::istream response_stream(&m_response);
		std::string hstr;

		while (std::getline(response_stream, hstr, '\n') && hstr != "\r"){
			size_t a;
			a = hstr.find(':');		
			Hydra::Header nh;
			nh.name = hstr.substr(0, a);
			nh.value = hstr.substr(a+2,hstr.length() - a - 2); 
			if(nh.name != "Connection" && nh.name != "Keep-Alive"){
				m_connection.reply().headers.push_back(nh);
			}
		}

		// Write whatever content we already have to output.
		if (m_response.size() > 0){
			std::stringstream ss;
			std::copy(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
			m_connection.reply().content(ss.str());
		}

		m_connection.perform_write();

		std::cout << "Async First Write Done. Resuming Read" << std::endl;


		// Start reading remaining data until EOF.
		boost::asio::async_read(m_socket, m_response,
				boost::asio::transfer_at_least(1),
				boost::bind(&Hydra::Apache2::Client::Connection::handle_read_content, this,
					boost::asio::placeholders::error));
	} else {
		std::cout << "Hydra: Apache2: Error: " << err << "\n";
	}
}

void Hydra::Apache2::Client::Connection::handle_read_content(const boost::system::error_code& err){

	std::cout << "Handle_Read" << std::endl;

	if (!err){

		// Write all of the data that has been read so far.
		if (m_response.size() > 0){
			std::istream response_stream(&m_response);
			std::stringstream ss;
			std::copy(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
			m_connection.reply().content(ss.str());
		}

//		m_connection.perform_write();

		std::cout << "Async Write Done. Resuming Read" << std::endl;

		// Continue reading remaining data until EOF.
		boost::asio::async_read(m_socket, m_response,
				boost::asio::transfer_at_least(1),
				boost::bind(&Hydra::Apache2::Client::Connection::handle_read_content, this,
					boost::asio::placeholders::error));
	} else if (err != boost::asio::error::eof){
		std::cout << "Hydra: Apache2: Error: " << err << "\n";
	} else {
		std::cout << "EOF" << std::endl;
	}

}

