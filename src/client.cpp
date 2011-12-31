//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "exception.hpp"

#include "client.hpp"

#include "client/plain.hpp"
#include "client/ssl.hpp"

Hydra::Client::Base::Base(std::string name, Config::Section config, Daemon& hydra) : m_config(config), m_hydra(hydra){
	
}

Hydra::Client::Base* Hydra::Client::Create(std::string name, Config::Section config, Daemon& hydra){

	if(config.value("type") == "plain"){

		return new Client::Plain(name, config, hydra);

	} else if(config.value("type") == "ssl"){

		return new Client::SSL(name, config, hydra);

	} else {

		throw new Exception(std::string("Hydra->Client->Unknown client type: ").append(config.value("type")));

	}

}

