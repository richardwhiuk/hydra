//
// hostmap.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "exception.hpp"
#include "hostmap.hpp"

Hydra::HostMap::HostMap() : m_wildcard(0), m_host(0) {

}

Hydra::HostMap::~HostMap(){

	for(std::map<std::string, HostMap*>::iterator it = m_children.begin(); it != m_children.end(); ++it){

		delete it->second;

	}

}

std::list<std::string> Hydra::HostMap::split(std::string hostname){

	size_t pos, x = 0;

	std::list<std::string> domain;

	while( (pos = hostname.find('.', x) ) != std::string::npos){

		domain.push_front(hostname.substr(x, pos - x));

		x = pos + 1;

	}

	domain.push_front(hostname.substr(x));

	return domain;

}

void Hydra::HostMap::add(std::string hostname, Server::Base* server){

	add(split(hostname), server);

}

void Hydra::HostMap::add(std::list<std::string> hostname, Server::Base* server){

	if(hostname.size() == 0){

		m_host = server;

		return;

	}

	if(hostname.size() == 1 && hostname.front() == "*"){

		m_wildcard = server;

	}

	std::string name = hostname.front();

	hostname.pop_front();

	std::map<std::string, Hydra::HostMap*>::iterator it = m_children.find(name);

	if(it == m_children.end()){

		m_children[name] = new HostMap();

	}

	m_children[name]->add(hostname, server);

}

Hydra::Server::Base* Hydra::HostMap::resolve(std::string hostname){

	try {

	return resolve(split(hostname));

	} catch(Exception* e){

		throw e;

	}

}

Hydra::Server::Base* Hydra::HostMap::resolve(std::list<std::string> hostname){

	if(hostname.size() == 0){

		if(m_host){

			return m_host;

		}

		throw new Exception("Hydra->HostMap->Failed to resolve hostname.");

	}

	std::string name = hostname.front();

	std::map<std::string, Hydra::HostMap*>::iterator it;

	it = m_children.find(name); 

	if(it != m_children.end()){

		try {

			hostname.pop_front();

			return it->second->resolve(hostname);

		} catch(Exception* e){

			delete e;

		}

	}

	if(m_wildcard){

		return m_wildcard;

	}

	throw new Exception("Hydra->HostMap->Failed to resolve hostname.");


}

