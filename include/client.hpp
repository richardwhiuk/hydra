//
// client.hpp
// ~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CLIENT_HPP
#define HYDRA_FILE_CLIENT_HPP

#include "config.hpp"

#include <string>

#include <boost/asio.hpp>

namespace Hydra {

class Daemon;

namespace Client {

class Base {

public:

	Base(std::string name, Config::Section config, Daemon& hydra);

	virtual void run(boost::asio::io_service& io_service) = 0;

	virtual ~Base() = 0;

protected:

	Config::Section m_config;

	Daemon& m_hydra;

};

Base* Create(std::string name, Config::Section config, Daemon& hydra); 

}

}

#endif

