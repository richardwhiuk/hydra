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

	std::list<std::string> split(std::string host);

	void add(std::string host, Server::Base* server);
	void add(std::list<std::string> host, Server::Base* server);

	Server::Base* resolve(std::string host);
	Server::Base* resolve(std::list<std::string> host);

private:

	Server::Base* m_wildcard;
	Server::Base* m_host;

	std::map<std::string, HostMap*> m_children;

};

}

#endif

