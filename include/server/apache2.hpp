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
#include "server/sslproxy.hpp"

#include <string>
#include <pwd.h>
#include <grp.h>

namespace Hydra {

class Daemon;

namespace Server {

class Apache2 : public Base {

public:
	Apache2(std::string name, Config::Section config, Daemon& hydra);
	virtual ~Apache2();

	void start();

	void reap(std::string reason);

	virtual void run(boost::asio::io_service& io_service);

	virtual void handle(Hydra::Connection::pointer connection);

	void release(Hydra::Connection* connection);

	void timeout(boost::posix_time::time_duration time);

	void handle_timeout(const boost::system::error_code& e);

	void cancel_timeout();

	void mkdirs();

private:

	void signal(std::string);

	static bool s_done;

	Proxy plain;
	SslProxy ssl;

	bool m_started;

	std::string m_config_file;

	std::string m_user;
	uid_t m_uid;

	std::string m_group;
	gid_t m_gid;

	int m_reap_timeout;	// Reaper timeout

	boost::asio::deadline_timer* m_timer;	// Reaper timer

	float m_load_1; // Maximum load at which we will start a new server
	float m_load_5;
	float m_load_15;

	unsigned long m_mem_start; // Amount of memory needed to start an Apache

	unsigned long m_swap_start; // Amount of swap needed

	unsigned long m_mem_reap; // Amount of memory needed to not reap an Apache

	unsigned long m_swap_reap; // Amount of swap need to not reap an Apache

        // Number of live connections
	uint32_t m_live;

};

}

}

