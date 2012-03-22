//
// ssl.hpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CLIENT_SSL
#define HYDRA_FILE_CLIENT_SSL

#include "client.hpp"
#include "client/plain.hpp"
#include "config.hpp"
#include "connection.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Hydra {

namespace Client {

class SSL : public Client::HTTP {

public:

	SSL(std::string name, Config::Section config, Daemon& hydra);
	virtual ~SSL();

	virtual void run(boost::asio::io_service&);

	class Socket : public HTTP::Socket {

	public:

		Socket(boost::asio::io_service& io, boost::asio::ssl::context& context) : _(io, context){

		}

		boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type& base(){
			return _.lowest_layer();
		}

		virtual void close(){
			_.lowest_layer().close();
		}

		virtual void cancel(){
			_.lowest_layer().cancel();
		}

		virtual void async_handshake(
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::handshake_type ht, 
			boost::function<void(const boost::system::error_code& ec)> rh){

			_.async_handshake(ht,rh);
		}

		virtual void async_read(
			boost::asio::mutable_buffers_1 mb, 
			boost::asio::detail::transfer_at_least_t cc, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh){

			boost::asio::async_read(_,mb,cc,rh);
		}

		virtual void async_write(
			boost::asio::const_buffers_1 cb, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh){

			boost::asio::async_write(_,cb,rh);
		}

		virtual void async_write(
			boost::asio::const_buffers_1 cb, 
			boost::asio::detail::transfer_at_least_t cc, 
			boost::function<void(const boost::system::error_code& ec, unsigned int bytes)> rh){

			boost::asio::async_write(_,cb,cc,rh);
		}

		virtual std::string remote_address(){
			return _.lowest_layer().remote_endpoint().address().to_string();
		}

	private:
		
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> _;
		

	};

	class Connection : public HTTP::Connection { //, public boost::enable_shared_from_this<Connection> {

	public:

		typedef boost::shared_ptr<Connection> pointer;

		static pointer Create(boost::asio::io_service& io_service, Daemon& hydra, boost::asio::ssl::context& context, std::string& tag);
		~Connection();

		void start();

		SSL::Socket& socket();

		boost::shared_ptr<Connection> shared_from_this(){
			return boost::static_pointer_cast<Connection>(HTTP::Connection::shared_from_this());
		}


	private:

		Connection(boost::asio::io_service& io_service, Daemon& hydra, boost::asio::ssl::context& context, std::string& tag);

		void handshake();

		void handle_handshake(const boost::system::error_code& e);

		SSL::Socket m_socket;

	};

protected:

	void accept();

	boost::asio::ssl::context* m_context;

};

}

}

#endif

