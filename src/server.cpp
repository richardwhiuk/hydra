//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"

#include "server/proxy.hpp"
#include "server/sslproxy.hpp"
#include "server/apache2.hpp"
#include "server/redirect.hpp"

#include "exception.hpp"

Hydra::Server::Base::Base(std::string name, Hydra::Config::Section config, Daemon& hydra) : m_hydra(hydra), m_name(name), m_config(config){

}

Hydra::Server::Base::~Base(){

}

Hydra::Server::Base* Hydra::Server::Create(std::string name, Hydra::Config::Section config, Hydra::Daemon& hydra){

	if(config.value("type") == "proxy"){

		return new Hydra::Server::Proxy(name, config, hydra);

	} else if (config.value("type") == "sslproxy"){

		return new Hydra::Server::SslProxy(name, config, hydra);

	} else if(config.value("type") == "apache2"){

		return new Hydra::Server::Apache2(name, config, hydra);

	} else if(config.value("type") == "redirect"){

		return new Hydra::Server::Redirect(name, config, hydra);

	} else {

		throw new Hydra::Exception(std::string("Hydra->Server->Unknown server type: ").append(config.value("type")));

	}

}

void Hydra::Server::Base::hosts(HostMap& map){

	std::list<std::string> hosts = m_config.values("host");

	std::list<std::string> tags = m_config.values("tag");

	for(std::list<std::string>::iterator it = hosts.begin(); it != hosts.end(); ++it){

		map.add(*it, tags, this);

	}

}

