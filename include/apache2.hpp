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

	virtual void request(Hydra::Connection&);

private:

	class Client {
		
	public:
		Client(const std::string&, const std::string&);

		const std::string& port();

		const std::string& server();

		void run(Hydra::Connection&);

		~Client();

	private:

		class Connection {

		public:
			Connection(boost::asio::io_service&, Hydra::Connection&, Apache2::Client& client);

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
			Hydra::Connection& m_connection;		// User -> Hydra connection
									// this = Hydra->Apache2 connection

		};

		const std::string m_server;
		const std::string m_port;

	};

	bool start();
	void mkdirs();
	bool signal(std::string signal);

	bool m_started;
	
	std::string m_user;
	std::string m_group;

	Apache2::Client m_client;

	uid_t m_uid;
	gid_t m_gid;

};

}

#endif

