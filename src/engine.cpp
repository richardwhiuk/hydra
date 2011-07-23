//
// engine.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "engine.hpp"
#include "apache2.hpp"

Hydra::Engine* Hydra::Engine::Create(std::string engine, Hydra::Config::Section& details, Hydra::Server* server){
	if(engine == "apache2")
		return new Apache2(details, server);

	return NULL;
}

bool Hydra::Engine::Valid(std::string engine){
	if(engine == "apache2")
		return true;
	
	return false;
}

