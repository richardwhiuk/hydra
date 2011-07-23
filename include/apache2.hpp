//
// apache2.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "engine.hpp"
#include <sys/types.h>

#ifndef HYDRA_FILE_APACHE2_HPP
#define HYDRA_FILE_APACHE2_HPP

namespace Hydra {

class Server;

class Apache2 : public Engine { 

public:

	Apache2(Config::Section& config, Server* server);

	virtual ~Apache2();

	virtual void request(const Hydra::request&, Hydra::reply&);

private:

	bool start();
	void mkdirs();
	bool signal(std::string signal);

	bool m_started;
	
	std::string m_user;
	std::string m_group;

	uid_t m_uid;
	gid_t m_gid;

};

}

#endif

