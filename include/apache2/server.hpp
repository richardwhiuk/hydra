//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "engine.hpp"
#include <sys/types.h>
#include <boost/asio.hpp>

#ifndef HYDRA_FILE_APACHE2_SERVER_HPP
#define HYDRA_FILE_APACHE2_SERVER_HPP

namespace Hydra {

namespace Apache2 {

class Server {

public:

	Server(Engine&);

	virtual ~Server();

	bool ready();

private:

	bool start();

	void mkdirs();
	bool signal(std::string signal);

	bool m_started;
	
	Hydra::Apache2::Engine& m_engine;

	std::string m_user;
	std::string m_group;

	uid_t m_uid;
	gid_t m_gid;

};

}

}

#endif


