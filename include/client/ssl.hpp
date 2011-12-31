//
// ssl.hpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "client.hpp"
#include "config.hpp"

#include <string>

namespace Hydra {

namespace Client {

class SSL : public Base {

public:

	SSL(std::string name, Config::Section config, Daemon& hydra);
	virtual ~SSL();

	virtual void run(boost::asio::io_service&);

};

}

}

