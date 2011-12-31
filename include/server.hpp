//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_SERVER_HPP
#define HYDRA_FILE_SERVER_HPP

#include "config.hpp"
#include "connection.hpp"
#include "hostmap.hpp"

#include <string>

#include <boost/asio.hpp>

namespace Hydra {

class Daemon;

namespace Server {

class Base {

public:

	Base(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Base();

	virtual void run(boost::asio::io_service& io_service) = 0;

	virtual void handle(Hydra::Connection::pointer connection) = 0;

	virtual void hosts(HostMap& map);

protected:

	Hydra::Daemon& m_hydra;
	std::string m_name;
	Config::Section m_config;

};

Base* Create(std::string name, Config::Section config, Daemon& hydra); 

}

}

#endif

