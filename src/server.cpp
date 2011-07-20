#include "server.hpp"

#include <iostream>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <pthread.h>
#include <signal.h>

using namespace std;

bool Hydra::Server::setup(){

	if(m_setup){
		return true;
	}

	cout << "Hydra: HTTP/1.1 Daemon" << endl;
	cout << "Hydra: Version " << HYDRA << "" << endl;
	cout << "Hydra: Config File: " << m_config.file() << endl;

	// Configuration

	if(!m_config.parse()){
		cerr << "Failed to parse configuration file." << std::endl;
		return false;
	}

	// Bind

	std::string address = m_config.get("global", "address");
	std::string port = m_config.get("global", "plain");

	if(address == "" || port == ""){
		cerr << "Hydra: Bind address is not set correctly" << std::endl;
		return false;
	}

	cout << "Hydra: Binding to " << address << ":" << port << endl;

	try {
		// Threads

		size_t threads = boost::lexical_cast<size_t>(m_config.get("global", "threads"));

		m_io_service_pool = Hydra::io_service_pool_ptr(new io_service_pool(threads));

		m_new_connection = Hydra::connection_ptr(new connection(
					m_io_service_pool->get_io_service(), 
					m_request_handler));

		// Connection Acceptor

		m_acceptor = new boost::asio::ip::tcp::acceptor(m_io_service_pool->get_io_service());
	
		boost::asio::ip::tcp::resolver resolver(m_acceptor->get_io_service());
		boost::asio::ip::tcp::resolver::query query(address, port);
		boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

		m_acceptor->open(endpoint.protocol());
		m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor->bind(endpoint);
		m_acceptor->listen();
		m_acceptor->async_accept(m_new_connection->socket(),
			boost::bind(
				&Server::handle_accept, 
				this,
				boost::asio::placeholders::error));

	} catch (std::exception& e) {

		std::cerr << "Hydra: Go() Exception: " << e.what() << "\n";

		return false;

	}

	// Setup complete

	m_setup = true;

	return true;

}

void Hydra::Server::run(){
	
	m_io_service_pool->run();

}

void Hydra::Server::stop(){

	m_io_service_pool->stop();

}

unsigned int Hydra::Server::go(){

	if(!setup()){
		std::cerr << "Hydra: Setup failed." << std::endl;
		return 3;
	}

	// Signal setup

	try {

		// Block all signals for background thread.
		sigset_t new_mask;
		sigfillset(&new_mask);
		sigset_t old_mask;
		pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

		// Run server in background thread.
		boost::thread t(boost::bind(&Hydra::Server::run, this));

		// Restore previous signals.
		pthread_sigmask(SIG_SETMASK, &old_mask, 0);

		// Wait for signal indicating time to shut down.
		sigset_t wait_mask;
		sigemptyset(&wait_mask);
		sigaddset(&wait_mask, SIGINT);
		sigaddset(&wait_mask, SIGQUIT);
		sigaddset(&wait_mask, SIGTERM);
		pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
		int sig = 0;
		sigwait(&wait_mask, &sig);

		// Stop the server.
		stop();
		t.join();

	} catch (std::exception& e) {

		std::cerr << "Hydra: Go() Exception: " << e.what() << "\n";

	}

}

void Hydra::Server::handle_accept(const boost::system::error_code& e)
{
	if (!e){
		m_new_connection->start();
		m_new_connection.reset(new connection(
			m_io_service_pool->get_io_service(), 
			m_request_handler));
		m_acceptor->async_accept(
			m_new_connection->socket(),
			boost::bind(
				&Server::handle_accept, 
				this, 
				boost::asio::placeholders::error));
	}
}

