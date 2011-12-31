//
// daemon.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "connection.hpp"
#include "hostmap.hpp"

#include <string>
#include <list>

namespace Hydra {

namespace Client {

class Base;

}

namespace Server {

class Base;

}

class Daemon {

public:

	Daemon();
	virtual ~Daemon();

	void configure(std::string file);
	void run();

	void handle(Connection::pointer connection);

private:

	// Global configuration

	Config m_config;

	// Client connection ports (e.g. SSL and none SSL)

	std::list<Client::Base*> m_clients;

	// Server connections (subdomains)

	std::list<Server::Base*> m_servers;

	// Host mapping

	HostMap m_hosts;

};

}

