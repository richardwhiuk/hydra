//
// apache2.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server/proxy.hpp"

#include <string>
#include <pwd.h>
#include <grp.h>

namespace Hydra {

class Daemon;

namespace Server {

class Apache2 : public Proxy {

public:
	Apache2(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Apache2();

	virtual void run(boost::asio::io_service& io_service);

	virtual void handle(Hydra::Connection::pointer connection);

	static void mkdirs();

private:

	void signal(std::string);

	static bool s_done;

	bool m_started;

	std::string m_config_file;

	std::string m_user;
	uid_t m_uid;

	std::string m_group;
	gid_t m_gid;

};

}

}

