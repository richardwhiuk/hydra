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
#include <apache2.hpp>
#include <apache2/server.hpp>

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

Hydra::Apache2::Server::Server(Hydra::Apache2::Engine& engine) : m_engine(engine){

	const char* ustr = "nobody";
	const char* gstr = "nogroup";

	if(m_engine.user().size() > 0){
		ustr = m_engine.user().front().c_str();
	}

	if(m_engine.group().size() > 0){
		gstr = m_engine.group().front().c_str();
	}

	{
		size_t buf = sysconf(_SC_GETPW_R_SIZE_MAX);
		struct passwd ustruct;
		char* buffer = new char[buf];
		struct passwd* udata;
		int result = getpwnam_r(ustr, &ustruct, buffer, buf, &udata);

		if(result != 0){
			std::cerr << "Hydra: Apache2: [" << m_engine.name() << "] User Data Lookup Failed." << std::endl;
		} 

		if(udata == 0){
			std::cerr << "Hydra: Apache2: [" << m_engine.name() << "] User Data Not Found." << std::endl;
			return;
		}

		m_uid = udata->pw_uid;
		m_user = udata->pw_name;

		delete[] buffer;
	}

	{
		size_t buf = sysconf(_SC_GETGR_R_SIZE_MAX);
		struct group gstruct;
		char* buffer = new char[buf];
		struct group* gdata;
		int result = getgrnam_r(gstr, &gstruct, buffer, buf, &gdata);

		if(result != 0){
			std::cerr << "Hydra: Apache2: [" << m_engine.name() << "] Group Data Lookup Failed." << std::endl;
		} 

		if(gdata == 0){
			std::cerr << "Hydra: Apache2: [" << m_engine.name() << "] Group Data Not Found." << std::endl;
			return;
		}

		m_gid = gdata->gr_gid;
		m_group = gdata->gr_name;

		delete[] buffer;

	}

	if(signal("start"))
		m_started = true;

}

bool Hydra::Apache2::Server::ready(){
	if(!m_started){
		return false;
	} 
	return true;
}

Hydra::Apache2::Server::~Server(){

	if(m_started){
		signal("stop");
	}

}

void Hydra::Apache2::Server::mkdirs(){
	if(mkdir("/var/run/hydra", 00775) && errno != EEXIST)
		_exit(-2);

	if(mkdir("/var/run/hydra/apache2", 00775) && errno != EEXIST)
		_exit(-2);

	{
		std::string rds = ("/var/run/hydra/apache2/" + m_engine.name());
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
	
		std::string lds = ("/var/log/hydra/apache2/" + m_engine.name());
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
	
		std::string lds = ("/var/lock/hydra/apache2/" + m_engine.name());
		const char* ldc = lds.c_str();
	
		if(mkdir(ldc, 00775) && errno != EEXIST)
			_exit(-2);
	
		if(chown(ldc, m_uid, m_gid))
			_exit(-2);

	}

}

bool Hydra::Apache2::Server::signal(std::string signal){
	pid_t pid = fork();

	switch(pid){

	case -1:
		return false;
		
	case 0: {

		m_engine.server().restore_signals();

		if(signal == "start"){
			mkdirs();
			setgid(m_gid);
			setuid(m_uid);
		}

		std::string apconfig("/etc/apache2/apache2.conf");

		if(m_engine.config().size() > 0){
			apconfig = m_engine.config().front();
		}

		char* newargv[10];

		const std::string name = m_engine.name();

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
