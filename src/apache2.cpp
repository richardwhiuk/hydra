//
// apache2.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "apache2.hpp"

#include <iostream>

#include <sys/types.h>
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

// TODO:
// We should have a mutex in host protecting engine creation...

Hydra::Apache2::Apache2(Config::Section& config) : Hydra::Engine::Engine(config){

	uid_t uid; 				// User of apache (assume nobody)
	const char* ustr = "nobody";

	gid_t gid;
	const char* gstr = "nogroup";

	if(config["user"].size() > 0){
		ustr = config["user"].front().c_str();
	}

	if(config["group"].size() > 0){
		gstr = config["group"].front().c_str();
	}

	// TODO:
	// We should put a mutex around this to prevent overwriting the data structure.

	struct passwd * udata = getpwnam(ustr);
	
	if(udata == 0){
		// Server Error
		return;
	}		

	uid = udata->pw_uid;
	std::string user = udata->pw_name;

	struct group * gdata = getgrnam(gstr);

	if(gdata == 0){
		// Server Error
		return;
	}

	gid = gdata->gr_gid;
	std::string group = gdata->gr_name;

	std::cout << "Starting Apache Server: (" << user << "," << group << ")" << std::endl;
	
	pid_t pid;				// Process of apache start daemon

	pid = fork();

	if(pid == -1){
		// Server error
		return;
	}

	if(pid == 0){
		
		setgid(gid);
		setuid(uid);

		// TODO: We should make sure that the folders exist as apache2ctl does this.
		// Each folder should be owned by the webserver user/group

		std::string apconfig("/etc/apache2/apache2.conf");

		if(config["config"].size() > 0){
			apconfig = config["config"].front();
		}

		char* newargv[10];

		strmasscpy(newargv, 0, "/usr/sbin/apache2");
		strmasscpy(newargv, 1, "-d");
		strmasscpy(newargv, 2, "/etc/apache2");
		strmasscpy(newargv, 3, "-k");
		strmasscpy(newargv, 4, "start");
		strmasscpy(newargv, 5, "-f");
		strmasscpy(newargv, 6, apconfig);
		strmasscpy(newargv, 7, "-D");
		strmasscpy(newargv, 8, "HYDRA");
		newargv[9] = NULL;

		char* newenviron[8];
		
		strmasscpy(newenviron, 0, "APACHE_CONFDIR=/etc/apache2");
		strmasscpy(newenviron, 1, "APACHE_RUN_USER=" + user);
		strmasscpy(newenviron, 2, "APACHE_RUN_GROUP=" + group);
		strmasscpy(newenviron, 3, "APACHE_PID_FILE=/var/run/hydra/apache2/" + config.name() + "/pid");
		strmasscpy(newenviron, 4, "APACHE_RUN_DIR=/var/run/hydra/apache2/" + config.name());
		strmasscpy(newenviron, 5, "APACHE_LOCK_DIR=/var/lock/hydra/apache2/" + config.name());
		strmasscpy(newenviron, 6, "APACHE_LOG_DIR=/var/log/hydra/apache2/" + config.name());
		newenviron[7] = NULL;

		execve("/usr/sbin/apache2", newargv, newenviron);

		_exit(0);

	} else {

		std::cout << "PID: " << pid << std::endl;

		// pid == Apache daemon proc.

	}

}

Hydra::Apache2::~Apache2(){

	// TODO: Stop apache.

}

void Hydra::Apache2::request(const Hydra::request&, Hydra::reply&){

}

