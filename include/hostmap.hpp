//
// hostmap.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_HOSTMAP_HPP
#define HYDRA_FILE_HOSTMAP_HPP

#include <map>
#include <string>
#include <list>

namespace Hydra {

namespace Server{

class Base;

}

class HostMap {

public:

	HostMap();
	~HostMap();

	std::list<std::string> split_domain(std::string host);

	void add(std::string host, std::list<std::string> tags, Server::Base* server);
	void add(std::list<std::string> host, std::list<std::string> tags, Server::Base* server);

	Server::Base* resolve(std::string host, std::string tag);
	Server::Base* resolve(std::list<std::string> host, std::string tag);

private:

	std::map<std::string, Server::Base*> m_wildcard;
	std::map<std::string, Server::Base*> m_host;

	std::map<std::string, HostMap*> m_children;

};

}

#endif

