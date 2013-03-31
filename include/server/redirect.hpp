//
// redirect.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2012-2012 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>

#include "server.hpp"

namespace Hydra {

class Daemon;

namespace Server {

class Redirect : public Base {

public:
	Redirect(std::string name, Config::Section config, Config::Section defaults, Daemon& hydra);

	virtual ~Redirect();

	virtual void run(boost::asio::io_service& io_service);

	virtual void handle(Hydra::Connection::pointer connection);

};

}

}

