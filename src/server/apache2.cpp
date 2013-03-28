//
// apache2.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server/apache2.hpp"

#include "daemon.hpp"
#include "utility.hpp"

#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

Hydra::Server::Apache2::Apache2(std::string name, Hydra::Config::Section config, Hydra::Daemon& daemon) : Base(name, config, daemon), plain(name, config, daemon), ssl(name, config, daemon), m_started(false){

	// Setup ready to start Apache2.

	// We do this here so we can inform the admin of errors prior to a eventual invocation.

	m_config_file = config.value("config");

	// Does the configuration file exist

	if(eaccess(m_config_file.c_str(), R_OK | F_OK)){
		throw new Exception("Hydra->Server->Apache->Configuration file not found [" + m_config_file + "]");
	}

	// Can we execute apache?

	if(eaccess("/usr/sbin/apache2", R_OK | F_OK | X_OK )){
		throw new Exception("Hydra->Server->Apache2->Can not call /usr/sbin/apache2");
	}

	// Get permissions data

	std::string user = config.value("user");
	std::string group = config.value("group");

	{
		size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

		if(bufsize == ((size_t) -1)){
			throw new Exception("Hydra->Server->Apache2->Unknown user buffer size");
		}

		char* buffer = new char[bufsize];

		struct passwd data;
		struct passwd* ptr;

		int result = getpwnam_r(user.c_str(), &data, buffer, bufsize, &ptr);

		if(result != 0){
			std::string message = "Hydra->Server->Apache2->User data lookup failed";
			throw new Exception(message + name);
		}

		if(ptr == 0){
			std::string message = "Hydra->Server->Apache2->User not found for: ";
			throw new Exception(message + name);
		}

		m_uid = ptr->pw_uid;
		m_user = ptr->pw_name;

		delete[] buffer;
	}

	{
		size_t bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);

		if(bufsize == ((size_t) -1)){
			throw new Exception("Hydra->Server->Apache2->Unknown group buffer size");
		}

		char* buffer = new char[bufsize];

		struct group data;
		struct group* ptr;

		int result = getgrnam_r(group.c_str(), &data, buffer, bufsize, &ptr);

		if(result != 0){
			throw new Exception("Hydra->Server->Apache2->Group data lookup failed");
		}

		if(ptr == 0){
			throw new Exception("Hydra->Server->Apache2->Group not found");
		}

		m_gid = ptr->gr_gid;
		m_group = ptr->gr_name;

		delete[] buffer;

	}

	// Make required directories

	mkdirs();

	// Check permissions of the directories

	{
		std::string dir = "/var/run/hydra/apache2/" + name;
		
		if(chown(dir.c_str(), m_uid, m_gid)){
			throw new Exception("Hydra->Server->Apache2->Failed to change directory permissions");
		}
	}

	{
		std::string dir = "/var/lock/hydra/apache2/" + name;
		
		if(chown(dir.c_str(), m_uid, m_gid)){
			throw new Exception("Hydra->Server->Apache2->Failed to change directory permissions");
		}
	}

	{
		std::string dir = "/var/log/hydra/apache2/" + name;
		
		if(chown(dir.c_str(), m_uid, m_gid)){
			throw new Exception("Hydra->Server->Apache2->Failed to change directory permissions");
		}
	}

}

Hydra::Server::Apache2::~Apache2(){

	// Shutdown Apache2 server

	if(m_started){
		signal("stop");
	}

}

void Hydra::Server::Apache2::run(boost::asio::io_service& io_service){

	plain.run(io_service);

	ssl.run(io_service);

}

void Hydra::Server::Apache2::handle(Hydra::Connection::pointer connection){

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
		throw new Exception("Hydra->Server->Apache2->Unknown connection type");
	}

}

void Hydra::Server::Apache2::signal(std::string signal){

	pid_t pid = fork();

	switch(pid){
		case -1:
			throw new Exception("Hydra->Server->Apache2->Failed to fork");

			break;

		case 0:
		{	
			m_hydra.restore_signals();
	
			setgid(m_gid);
			setuid(m_uid);
			
			char* newargv[10];

			strmasscpy(newargv, 0, "/usr/sbin/apache2");
			strmasscpy(newargv, 1, "-k");
			strmasscpy(newargv, 2, signal);
			strmasscpy(newargv, 3, "-d");
			strmasscpy(newargv, 4, "/etc/apache2");
			strmasscpy(newargv, 5, "-f");
			strmasscpy(newargv, 6, m_config_file);
			strmasscpy(newargv, 7, "-D");
			strmasscpy(newargv, 8, "HYDRA");
			newargv[9] = NULL;	

			char* newenviron[8];

			strmasscpy(newenviron, 0, "APACHE_CONFDIR=/etc/apache2");
			strmasscpy(newenviron, 1, "APACHE_RUN_USER=" + m_user);
			strmasscpy(newenviron, 2, "APACHE_RUN_GROUP=" + m_group);
			strmasscpy(newenviron, 3, "APACHE_PID_FILE=/var/run/hydra/apache2/" + m_name + "/pid");
			strmasscpy(newenviron, 4, "APACHE_RUN_DIR=/var/run/hydra/apache2/" + m_name);
			strmasscpy(newenviron, 5, "APACHE_LOCK_DIR=/var/lock/hydra/apache2/" + m_name);
			strmasscpy(newenviron, 6, "APACHE_LOG_DIR=/var/log/hydra/apache2/" + m_name);
			newenviron[7] = NULL;

			execve("/usr/sbin/apache2", newargv, newenviron);

			_exit(0);
		}
		default:
		{
			int status;
			waitpid(pid, &status, 0);
			if(status != 0){
				throw new Exception("Hydra->Server->Apache->Signal Failed");
			}
		}
	}


}

bool Hydra::Server::Apache2::s_done = false;

void Hydra::Server::Apache2::mkdirs(){

	std::vector<std::string> dirs;

	if(!s_done){

		dirs.push_back("/var/run/hydra");
		dirs.push_back("/var/run/hydra/apache2");
		dirs.push_back("/var/lock/hydra");
		dirs.push_back("/var/lock/hydra/apache2");
		dirs.push_back("/var/log/hydra");
		dirs.push_back("/var/log/hydra/apache2");

		s_done = true;

	}

	size_t start = 0;
	size_t pos;

	do {

		pos = m_name.find_first_of('/', start);

		dirs.push_back(std::string("/var/run/hydra/apache2/" + m_name.substr(0, pos)));
		dirs.push_back(std::string("/var/lock/hydra/apache2/" + m_name.substr(0, pos)));
		dirs.push_back(std::string("/var/log/hydra/apache2/" + m_name.substr(0, pos)));

		start = pos + 1;
	
	} while(pos != std::string::npos);

	for(std::vector<std::string>::iterator it = dirs.begin(); it != dirs.end(); ++it){
		if(mkdir(it->c_str(), 00775) && errno != EEXIST){
			throw new Exception(std::string("Hydra->Server->Apache2->Failed to create directory [") + *it + "]");
		}
	}

}

