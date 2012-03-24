//
// nginx.cpp
// ~~~~~~~~~
//
// Copyright (c) 2012-2012 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server/nginx.hpp"

#include "daemon.hpp"
#include "utility.hpp"

#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

Hydra::Server::Nginx::Nginx(std::string name, Hydra::Config::Section config, Hydra::Daemon& daemon) : Base(name, config, daemon), plain(name, config, daemon), ssl(name, config, daemon), m_started(false){

	// Setup ready to start Nginx.

	// We do this here so we can inform the admin of errors prior to a eventual invocation.

	m_config_file = config.value("config");

	// Does the configuration file exist

	if(eaccess(m_config_file.c_str(), R_OK | F_OK)){
		throw new Exception("Hydra->Server->Nginx->Configuration file not found");
	}

	// Can we execute apache?

	if(eaccess("/usr/sbin/nginx", R_OK | F_OK | X_OK )){
		throw new Exception("Hydra->Server->Nginx->Can not call /usr/sbin/nginx");
	}

	// Get permissions data

	std::string user = config.value("user");
	std::string group = config.value("group");

	{
		size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

		if(bufsize == ((size_t) -1)){
			throw new Exception("Hydra->Server->Nginx->Unknown user buffer size");
		}

		char* buffer = new char[bufsize];

		struct passwd data;
		struct passwd* ptr;

		int result = getpwnam_r(user.c_str(), &data, buffer, bufsize, &ptr);

		if(result != 0){
			throw new Exception("Hydra->Server->Nginx->User data lookup failed");
		}

		if(ptr == 0){
			throw new Exception("Hydra->Server->Nginx->User not found");
		}

		m_uid = ptr->pw_uid;
		m_user = ptr->pw_name;

		delete[] buffer;
	}

	{
		size_t bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);

		if(bufsize == ((size_t) -1)){
			throw new Exception("Hydra->Server->Nginx->Unknown group buffer size");
		}

		char* buffer = new char[bufsize];

		struct group data;
		struct group* ptr;

		int result = getgrnam_r(group.c_str(), &data, buffer, bufsize, &ptr);

		if(result != 0){
			throw new Exception("Hydra->Server->Nginx->Group data lookup failed");
		}

		if(ptr == 0){
			throw new Exception("Hydra->Server->Nginx->Group not found");
		}

		m_gid = ptr->gr_gid;
		m_group = ptr->gr_name;

		delete[] buffer;

	}

}

Hydra::Server::Nginx::~Nginx(){

	// Shutdown Nginx server

	if(m_started){
		signal("stop");
	}

}

void Hydra::Server::Nginx::run(boost::asio::io_service& io_service){

	plain.run(io_service);

	ssl.run(io_service);

}

void Hydra::Server::Nginx::handle(Hydra::Connection::pointer connection){

	// If we aren't yet running the server, we really should.

	if(!m_started){

		signal("start");

		m_started = true;

	}

	std::string type = m_config.value_tag("connection", connection->tag());

	if(type == "plain"){
		plain.handle(connection);
	} else if(type == "ssl"){
		ssl.handle(connection);
	} else {
		throw new Exception("Hydra->Server->Nginx->Unknown connection type");
	}

}

void Hydra::Server::Nginx::signal(std::string signal){

	pid_t pid = fork();

	switch(pid){
		case -1:
			throw new Exception("Hydra->Server->Nginx->Failed to fork");

			break;

		case 0:
		{	
			m_hydra.restore_signals();
	
			setgid(m_gid);
			setuid(m_uid);
			
			if(signal != "start"){

				char* newargv[6];
				strmasscpy(newargv, 0, "/usr/sbin/nginx");
				strmasscpy(newargv, 1, "-c");
				strmasscpy(newargv, 2, m_config_file);
				strmasscpy(newargv, 3, "-s");
				strmasscpy(newargv, 4, signal);
				newargv[5] = NULL;

				execv("/usr/sbin/nginx", newargv);

			} else {

				char* newargv[4];
				strmasscpy(newargv, 0, "/usr/sbin/nginx");
				strmasscpy(newargv, 1, "-c");
				strmasscpy(newargv, 2, m_config_file);
				newargv[3] = NULL;

				execv("/usr/sbin/nginx", newargv);

			}

			_exit(1);
		}
		default:
		{
			int status;
			waitpid(pid, &status, 0);
			if(status != 0){
				throw new Exception("Hydra->Server->Nginx->Signal Failed");
			}
		}
	}


}

