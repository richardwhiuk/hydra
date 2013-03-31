//
// redirect.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2012-2012 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server/redirect.hpp"

#include "daemon.hpp"

#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

Hydra::Server::Redirect::Redirect(std::string name, Hydra::Config::Section config, Hydra::Config::Section defaults, Hydra::Daemon& daemon) : Base(name, config, defaults, daemon){

}

Hydra::Server::Redirect::~Redirect(){

}

void Hydra::Server::Redirect::run(boost::asio::io_service& io_service){

}

void Hydra::Server::Redirect::handle(Hydra::Connection::pointer connection){

	std::string type = "see_other";

	std::string location = m_config.value_tag("location", connection->tag());

	try {

		type = m_config.value_tag("redirect", connection->tag());

	} catch(Exception* e){

	}

	if(connection->request().version() == "1.1"){

		connection->response().version("1.1");

		if(type == "permenant"){

			connection->response().code(301);

		} else if(type == "temporary"){

			connection->response().code(307);

		} else {

			connection->response().code(303);

		}

	} else {

		if(type == "permenant"){

			connection->response().code(301);
	
		} else {

			connection->response().code(302);

		}

	}

	connection->response().header("Server", "Hydra");

	connection->response().header("Location", location);

	connection->response().done();

	connection.reset();

}

