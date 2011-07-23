//
// apache2.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "engine.hpp"
#include <sys/types.h>
#include <boost/asio.hpp>

#ifndef HYDRA_FILE_APACHE2_HPP
#define HYDRA_FILE_APACHE2_HPP

namespace Hydra {

class Server;

class Apache2 : public Engine { 

public:

	Apache2(Config::Section& config, Server* server);

	virtual ~Apache2();

	virtual void request(const Hydra::request&, Hydra::reply&);

private:
	
	class Client {
		
	public:
		Client(boost::asio::io_service&, const Hydra::request&, Hydra::reply&, const std::string&, const std::string&);

		~Client();

	private:

		void handle_read_content(const boost::system::error_code& err);
		void handle_read_headers(const boost::system::error_code& err);
		void handle_read_status_line(const boost::system::error_code& err);
		void handle_write_request(const boost::system::error_code& err);
		void handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
		void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		Hydra::reply& m_reply;

		boost::asio::ip::tcp::resolver m_resolver;
		boost::asio::ip::tcp::socket m_socket;
		boost::asio::streambuf m_request;
		boost::asio::streambuf m_response;

	};

	bool start();
	void mkdirs();
	bool signal(std::string signal);

	bool m_started;
	
	std::string m_user;
	std::string m_group;

	uid_t m_uid;
	gid_t m_gid;

};

}

#endif

