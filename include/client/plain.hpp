//
// plain.hpp
// ~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CLIENT_PLAIN
#define HYDRA_FILE_CLIENT_PLAIN

#include "client.hpp"
#include "config.hpp"

#include "connection.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

namespace Hydra {

namespace Client {

class Plain : public Base {

public:

	Plain(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Plain();

	virtual void run(boost::asio::io_service& io_service);

	class Connection : public boost::enable_shared_from_this<Connection> {

	public:

		typedef boost::shared_ptr<Connection> pointer;

		static pointer Create(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag);
		~Connection();

		void start();
	
		boost::asio::ip::tcp::socket& socket();
	
	private:

		Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag);

		void read();

		void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);

		void consume();

		void write_start();

		void write();

		void handle_write(const boost::system::error_code& e, std::size_t bytes_transferred);

		void finish();

		void handle_finish(const boost::system::error_code& e, std::size_t bytes_transferred);

		Hydra::Connection::pointer m_connection;

		Daemon& m_hydra;

		boost::array<char, 8192> m_buffer_in;

		size_t m_bytes_start; // Position in buffer

		size_t m_bytes_total; // Valid bytes in buffer

		std::string m_buffer_out;

		std::string& m_tag;

		boost::asio::ip::tcp::socket m_socket;

		bool m_persistent; // Persistent connection
	
	};

private:

	void accept();

	void handle(Connection::pointer connect, const boost::system::error_code& error);

	boost::asio::ip::tcp::acceptor* m_accept;

	std::string m_tag;

};

}

}

#endif

