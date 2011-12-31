//
// daemon.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daemon.hpp"
#include "client.hpp"
#include "server.hpp"

#include <boost/asio.hpp>

Hydra::Daemon::Daemon(){

}

Hydra::Daemon::~Daemon(){
	
	for(std::list<Client::Base*>::iterator it = m_clients.begin(); it != m_clients.end(); ++it){
		delete *it;
	}

	for(std::list<Server::Base*>::iterator it = m_servers.begin(); it != m_servers.end(); ++it){
		delete *it;
	}

}

void Hydra::Daemon::configure(std::string file){

	m_config.open(file);

	std::map<std::string, Config::Section> sections = m_config.sections();

	for(std::map<std::string, Config::Section>::iterator it = sections.begin(); it != sections.end(); ++it){

		size_t colon = it->first.find_first_of(':');

		if(colon != std::string::npos){
			
			std::string type = it->first.substr(0, colon);

			if(type == "client"){

				m_clients.push_front(Client::Create(it->first.substr(colon+1), it->second, *this));

			} else if (type == "server"){

				m_servers.push_front(Server::Create(it->first.substr(colon+1), it->second, *this));

			}

		}

	}

}

void Hydra::Daemon::run(){

	// Run Hydra

	boost::asio::io_service io_service;

	for(std::list<Client::Base*>::iterator it = m_clients.begin(); it != m_clients.end(); ++it){

		(*it)->run(io_service); 

	}

	for(std::list<Server::Base*>::iterator it = m_servers.begin(); it != m_servers.end(); ++it){

		(*it)->hosts(m_hosts);

		(*it)->run(io_service); 

	}

	
	/**

	TODO: This code only works in Boost 1.47

	if(daemon){

		boost::asio::signal_set signals_stop(io_service, SIGINT, SIGTERM);
		signals_stop.async_wait(boost::bind(&Hydra::Daemon::stop, this));
		io_service.notify_fork(boost::asio::io_service::fork_prepare);

		if(pid_t pid = fork()){
			if(pid > 0){
				exit(0);
			}
		} else {
			return 1;
		}

		setsid();
		chdir("/");
		umask(0);

		if (pid_t pid = fork()){
			if (pid > 0){
				exit(0);
			}
		} else {
			return 1;
		}

		close(0);
		close(1);
		close(2);

		io_service.notify_fork(boost::asio::io_service::fork_child);

	}

	**/

	io_service.run();

}

void Hydra::Daemon::handle(Hydra::Connection::pointer connection){

	m_hosts.resolve( connection->request().header("Host") )->handle(connection);

}

