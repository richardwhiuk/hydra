//
// apache2.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include "apache2.hpp"

#include "errno.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

// If only execve was happy with const strings. 
// Here's a horrible hack to get round it - we don't get about delete because we going to execve anyway.
// Oh well.

void strmasscpy(char** dst, size_t place, const std::string src){
	dst[place] = new char[src.size() + 1];
	std::strcpy(dst[place], src.c_str());
}

Hydra::Apache2::Apache2(Config::Section& config, Server* server) : Hydra::Engine::Engine(config, server){
	m_started = start();
}

bool Hydra::Apache2::start(){

	const char* ustr = "nobody";
	const char* gstr = "nogroup";

	if(m_details["user"].size() > 0){
		ustr = m_details["user"].front().c_str();
	}

	if(m_details["group"].size() > 0){
		gstr = m_details["group"].front().c_str();
	}

	// TODO:
	// We should put a mutex around this to prevent overwriting the data structure.

	struct passwd * udata = getpwnam(ustr);
	
	if(udata == 0){
		std::cerr << "Hydra: Apache2: [" << m_details.name() << "] User Data Not Found." << std::endl;
		return false;
	}		

	m_uid = udata->pw_uid;
	m_user = udata->pw_name;

	struct group * gdata = getgrnam(gstr);

	if(gdata == 0){
		std::cerr << "Hydra: Apache2: [" << m_details.name() << "] Group Data Not Found." << std::endl;
		return false;
	}

	m_gid = gdata->gr_gid;
	m_group = gdata->gr_name;

	return signal("start");

}

Hydra::Apache2::~Apache2(){

	if(m_started){
		signal("stop");
	}

}

void Hydra::Apache2::request(const Hydra::request& req, Hydra::reply& rep){

	if(!m_started){
		rep = reply::stock_reply(reply::service_unavailable);
		return;
	} 

	if(m_details["address"].size() == 0){
		rep = reply::stock_reply(reply::service_unavailable);
		return;
	}

	if(m_details["port"].size() == 0){
		rep = reply::stock_reply(reply::service_unavailable);
		return;
	}

	boost::asio::io_service io_service;

	Hydra::Apache2::Client client(
		io_service, 
		req,
		rep,
		m_details["address"].front(),
		m_details["port"].front()
	);

	io_service.run();

}

void Hydra::Apache2::mkdirs(){
	if(mkdir("/var/run/hydra", 00775) && errno != EEXIST)
		_exit(-2);

	if(mkdir("/var/run/hydra/apache2", 00775) && errno != EEXIST)
		_exit(-2);

	{
		std::string rds = ("/var/run/hydra/apache2/" + m_details.name());
		const char* rdc = rds.c_str();

		if(mkdir(rdc, 00775) && errno != EEXIST)
			_exit(-2);

		if(chown(rdc, m_uid, m_gid))
			_exit(-2);

	}

	if(mkdir("/var/log/hydra", 00775) && errno != EEXIST)
		_exit(-2);

	if(mkdir("/var/log/hydra/apache2", 00775) && errno != EEXIST)
		_exit(-3);

	{
	
		std::string lds = ("/var/log/hydra/apache2/" + m_details.name());
		const char* ldc = lds.c_str();
	
		if(mkdir(ldc, 00775) && errno != EEXIST)
			_exit(-2);
	
		if(chown(ldc, m_uid, m_gid))
			_exit(-2);
		
	}
		
	if(mkdir("/var/lock/hydra", 00775) && errno != EEXIST)
		_exit(-2);
	
	if(mkdir("/var/lock/hydra/apache2", 00775) && errno != EEXIST)
		_exit(-2);
	
	{
	
		std::string lds = ("/var/lock/hydra/apache2/" + m_details.name());
		const char* ldc = lds.c_str();
	
		if(mkdir(ldc, 00775) && errno != EEXIST)
			_exit(-2);
	
		if(chown(ldc, m_uid, m_gid))
			_exit(-2);

	}

}

bool Hydra::Apache2::signal(std::string signal){
	pid_t pid = fork();

	switch(pid){

	case -1:
		return false;
		
	case 0: {

		m_server->restore_signals();

		if(signal == "start"){
			mkdirs();
			setgid(m_gid);
			setuid(m_uid);
		}

		std::string apconfig("/etc/apache2/apache2.conf");

		if(m_details["config"].size() > 0){
			apconfig = m_details["config"].front();
		}

		char* newargv[10];

		const std::string name = m_details.name();

		strmasscpy(newargv, 0, "/usr/sbin/apache2");
		strmasscpy(newargv, 1, "-k");
		strmasscpy(newargv, 2, signal);
		strmasscpy(newargv, 3, "-d");
		strmasscpy(newargv, 4, "/etc/apache2");
		strmasscpy(newargv, 5, "-f");
		strmasscpy(newargv, 6, apconfig);
		strmasscpy(newargv, 7, "-D");
		strmasscpy(newargv, 8, "HYDRA");
		newargv[9] = NULL;

		char* newenviron[8];
		
		strmasscpy(newenviron, 0, "APACHE_CONFDIR=/etc/apache2");
		strmasscpy(newenviron, 1, "APACHE_RUN_USER=" + m_user);
		strmasscpy(newenviron, 2, "APACHE_RUN_GROUP=" + m_group);
		strmasscpy(newenviron, 3, "APACHE_PID_FILE=/var/run/hydra/apache2/" + name + "/pid");
		strmasscpy(newenviron, 4, "APACHE_RUN_DIR=/var/run/hydra/apache2/" + name);
		strmasscpy(newenviron, 5, "APACHE_LOCK_DIR=/var/lock/hydra/apache2/" + name);
		strmasscpy(newenviron, 6, "APACHE_LOG_DIR=/var/log/hydra/apache2/" + name);
		newenviron[7] = NULL;

		execve("/usr/sbin/apache2", newargv, newenviron);

		_exit(0);

	}
	default:
		int status;
		waitpid(pid, &status, 0);
		return (status == 0);
	}
}

