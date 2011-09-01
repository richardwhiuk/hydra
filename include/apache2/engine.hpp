//
// engine.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_APACHE2_ENGINE_HPP
#define HYDRA_FILE_APACHE2_ENGINE_HPP

#include <engine.hpp>
#include <server.hpp>
#include <apache2/client.hpp>
#include <apache2/server.hpp>

#include <sys/types.h>
#include <boost/asio.hpp>

namespace Hydra {

namespace Apache2 {

class Engine : public Hydra::Engine { 

public:

	Engine(Config::Section& config, Hydra::Server& server);

	virtual ~Engine();

	bool ready();

	virtual void request(Hydra::Connection::Ptr);

	const Hydra::Server& server();

// Configuration

	const std::string& name();			// Name in hydra.conf
	
	const std::vector<std::string>& config();	// Apache2 Conf File

	const std::vector<std::string>& user();		// Runas:
	const std::vector<std::string>& group();

	const std::vector<std::string>& address();	// Bind:
	const std::vector<std::string>& port();


private:

	Apache2::Server m_server;
	Apache2::Client m_client;

};

}

}

#endif


