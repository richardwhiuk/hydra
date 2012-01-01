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

Hydra::HostMap::HostMap(){

}

Hydra::HostMap::~HostMap(){

	for(std::map<std::string, HostMap*>::iterator it = m_children.begin(); it != m_children.end(); ++it){

		delete it->second;

	}

}

std::list<std::string> Hydra::HostMap::split_domain(std::string hostname){

	size_t pos, x = 0;

	std::list<std::string> domain;

	while( (pos = hostname.find('.', x) ) != std::string::npos){

		domain.push_front(hostname.substr(x, pos - x));

		x = pos + 1;

	}

	domain.push_front(hostname.substr(x));

	return domain;

}

void Hydra::HostMap::add(std::string hostname, std::list<std::string> tags, Server::Base* server){

	add(split_domain(hostname), tags, server);

}

void Hydra::HostMap::add(std::list<std::string> hostname, std::list<std::string> tags, Server::Base* server){

	if(hostname.size() == 0){

		for(std::list<std::string>::iterator it = tags.begin(); it != tags.end(); ++it){

			m_host[*it] = server;

		}

		return;

	}

	if(hostname.size() == 1 && hostname.front() == "*"){

		for(std::list<std::string>::iterator it = tags.begin(); it != tags.end(); ++it){

			m_wildcard[*it] = server;

		}

	}

	std::string name = hostname.front();

	hostname.pop_front();

	std::map<std::string, Hydra::HostMap*>::iterator it = m_children.find(name);

	if(it == m_children.end()){

		m_children[name] = new HostMap();

	}

	m_children[name]->add(hostname, tags, server);

}

Hydra::Server::Base* Hydra::HostMap::resolve(std::string hostname, std::string tag){

	try {

		return resolve(split_domain(hostname), tag);

	} catch(Exception* e){

		throw e;

	}

}

Hydra::Server::Base* Hydra::HostMap::resolve(std::list<std::string> hostname, std::string tag){

	if(hostname.size() == 0){

		std::map<std::string, Server::Base*>::iterator hit = m_host.find(tag);

		if(hit != m_host.end()){

			return hit->second;

		}

		throw new Exception("Hydra->HostMap->Failed to resolve hostname.");

	}

	std::string name = hostname.front();

	std::map<std::string, Hydra::HostMap*>::iterator it;

	it = m_children.find(name); 

	if(it != m_children.end()){

		try {

			hostname.pop_front();

			return it->second->resolve(hostname, tag);

		} catch(Exception* e){

			delete e;

		}

	}

	std::map<std::string, Server::Base*>::iterator wit = m_wildcard.find(tag);

	if(wit != m_wildcard.end()){

		return wit->second;

	}

	throw new Exception("Hydra->HostMap->Failed to resolve hostname.");


}

