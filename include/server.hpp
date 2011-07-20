#ifndef HYDRA_FILE_HYDRA
#define HYDRA_FILE_HYDRA

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "io_service_pool.hpp"
#include "request_handler.hpp"
#include "config.hpp"
#include "version.hpp"

namespace Hydra {

class Server : private boost::noncopyable {

public:

	Server(std::string& config) : m_setup(false), m_config(config){
	
	}

	bool setup();

	void run();

	unsigned int go();

	void stop();

	~Server(){

	}

private:

	/// Handle completion of an asynchronous accept operation.
	void handle_accept(const boost::system::error_code& e);

	bool m_setup;

	/// Hydra Configuration
	Hydra::Config m_config;

	/// The pool of io_service objects used to perform asynchronous operations.
	Hydra::io_service_pool_ptr m_io_service_pool;

	/// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor* m_acceptor;

	/// The next connection to be accepted.
	Hydra::connection_ptr m_new_connection;

	/// The handler for all incoming requests.
	Hydra::request_handler m_request_handler;

};

}

#endif

