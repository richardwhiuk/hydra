//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <connection.hpp>
#include <apache2/client.hpp>
#include <sys/types.h>
#include <boost/asio.hpp>

#ifndef HYDRA_FILE_APACHE2_CONNECTION_HPP
#define HYDRA_FILE_APACHE2_CONNECTION_HPP

namespace Hydra {

namespace Apache2 {

class Connection {

public:

	Connection(boost::asio::io_service&, Hydra::Connection::Ptr, Apache2::Client& client);

	~Connection();

private:

	void handle_read_content(const boost::system::error_code& err);
	void handle_read_headers(const boost::system::error_code& err);
	void handle_read_status_line(const boost::system::error_code& err);
	void handle_write_request(const boost::system::error_code& err);
	void handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	
	boost::asio::ip::tcp::resolver m_resolver;
	boost::asio::ip::tcp::socket m_socket;
	boost::asio::streambuf m_request;
	boost::asio::streambuf m_response;
		
	Apache2::Client& m_client;
	Hydra::Connection::Ptr m_connection;		// User -> Hydra connection
							// this = Hydra->Apache2 connection

};

}

}

#endif


