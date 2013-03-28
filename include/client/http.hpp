//
// http.hpp
// ~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CLIENT_HTTP
#define HYDRA_FILE_CLIENT_HTTP

#include "client.hpp"
#include "config.hpp"

#include "connection.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Hydra {

namespace Client {

class HTTP : public Base {

public:

	HTTP(std::string name, Config::Section config, Daemon& hydra);

	virtual void run(boost::asio::io_service& io_service);

	class Socket {

	public:

		Socket(){

		}

		virtual std::string remote_address() = 0;

		virtual void async_write(
			boost::asio::const_buffers_1 cb, 
			boost::asio::detail::transfer_at_least_t cc, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh) = 0;

		virtual void async_write(
			boost::asio::const_buffers_1 cb, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh) = 0;

		virtual void async_read(
			boost::asio::mutable_buffers_1 mb, 
			boost::asio::detail::transfer_at_least_t cc, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh) = 0;

		virtual void close() = 0;
		virtual void cancel() = 0;

	private:

	};

	class Connection :  public boost::enable_shared_from_this<Connection> {

	public:

		typedef boost::shared_ptr<Connection> pointer;

		virtual HTTP::Socket& socket() = 0;

		virtual void start();

	protected:

		Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag);

		virtual ~Connection();

		virtual void begin();

		virtual void read();

		virtual void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);

		virtual void consume();

		virtual void write_start();

		virtual void write();

		virtual void write_data();

		virtual void handle_write(const boost::system::error_code& e, std::size_t bytes_transferred);

		virtual void finish();

		virtual void handle_finish(const boost::system::error_code& e, std::size_t bytes_transferred);

		void timeout(boost::posix_time::time_duration time);

		void handle_timeout(const boost::system::error_code& e);

		Daemon& m_hydra;

		std::string& m_tag;

		Hydra::Connection::pointer m_connection;

		boost::array<char, 8192> m_buffer_in;

		size_t m_bytes_start; // Position in buffer

		size_t m_bytes_total; // Valid bytes in buffer

		std::string m_buffer_out;

		bool m_persistent; // Persistent connection

		boost::asio::deadline_timer m_timer;	// Prevent client's from holding on to sockets forever.

		int m_read_timeout;	// Read timeout

		int m_write_timeout;	// Write timeout

		std::string m_address;  // Client address

	};

protected:

	virtual void handle(Connection::pointer connect, const boost::system::error_code& error);

	virtual void accept() = 0;

	boost::asio::ip::tcp::acceptor* m_accept;

	std::string m_tag;

};

}

}

#endif

