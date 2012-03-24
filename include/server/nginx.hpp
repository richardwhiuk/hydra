//
// nginx.hpp
// ~~~~~~~~~
//
// Copyright (c) 2012-2012 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include "server/proxy.hpp"
#include "server/sslproxy.hpp"

#include <string>
#include <pwd.h>
#include <grp.h>

namespace Hydra {

class Daemon;

namespace Server {

class Nginx : public Base {

public:
	Nginx(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Nginx();

	virtual void run(boost::asio::io_service& io_service);

	virtual void handle(Hydra::Connection::pointer connection);

private:

	void signal(std::string);

	Proxy plain;
	SslProxy ssl;

	bool m_started;

	std::string m_config_file;

	std::string m_user;
	uid_t m_uid;

	std::string m_group;
	gid_t m_gid;

};

}

}

