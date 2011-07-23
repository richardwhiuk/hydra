//
// host.hpp
// ~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "engine.hpp"
#include "request.hpp"
#include "reply.hpp"

#include <boost/thread/mutex.hpp>

#ifndef HYDRA_FILE_HOST_HPP
#define HYDRA_FILE_HOST_HPP

namespace Hydra {

class Server;

class Host { 

public:

	explicit Host(Config::Section details, Server* server);

	typedef std::vector<std::string> Aliases;

	bool valid();

	std::vector<std::string> alias();

	void request(const request& req, reply& rep);

	~Host();

private:

	Config::Section m_details;
	Engine* volatile m_engine;	// Can be modified by a different thread.
	Server* m_server;
	boost::mutex m_engine_mux;

};

}


#endif

