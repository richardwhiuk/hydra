//
// client.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_APACHE2_CLIENT_HPP
#define HYDRA_FILE_APACHE2_CLIENT_HPP

#include <sys/types.h>
#include <boost/asio.hpp>
#include <apache2.hpp>
#include <apache2/connection.hpp>

namespace Hydra {

namespace Apache2 {

class Client {
		
public:
	Client(Hydra::Apache2::Engine& engine);

	const std::string& port();

	const std::string& server();

	bool ready();

	Hydra::Apache2::Connection::Ptr connection();

	~Client();

private:

	Hydra::Apache2::Engine& m_engine;
	
	const std::string m_server;
	const std::string m_port;

};

}

}

#endif

