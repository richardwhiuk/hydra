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
#include "client/http.hpp"
#include "config.hpp"

#include "connection.hpp"

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Hydra {

namespace Client {

class Plain : public Client::HTTP {

public:

	Plain(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Plain();

	class Socket : public HTTP::Socket {

	public:

		Socket(boost::asio::io_service& io) : _(io){

		}

		boost::asio::ip::tcp::socket& base(){
			return _;
		}

		virtual void close(){
			_.close();
		}

		virtual void cancel(){
			_.cancel();
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
			try {
				if(_.remote_endpoint().address().is_v4()){
					return _.remote_endpoint().address().to_v4().to_string();
				} else if(_.remote_endpoint().address().is_v6()){
					return _.remote_endpoint().address().to_v6().to_string();
				}	
			} catch(boost::system::system_error& e){
				
			}

			return "";	// Error resolving - abort.
		}

	private:

		boost::asio::ip::tcp::socket _;


	};

	class Connection : public HTTP::Connection {

	public:

		typedef boost::shared_ptr<Connection> pointer;

		static pointer Create(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag);
		~Connection();

		virtual Plain::Socket& socket();

	protected:

		Connection(boost::asio::io_service& io_service, Daemon& hydra, std::string& tag);

		Plain::Socket m_socket;
	
	};

protected:

	virtual void accept();

};

}

}

#endif

