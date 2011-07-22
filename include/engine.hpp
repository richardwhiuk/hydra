//
// engine.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "request.hpp"
#include "reply.hpp"

#ifndef HYDRA_FILE_ENGINE_HPP
#define HYDRA_FILE_ENGINE_HPP

namespace Hydra {

class Engine { 

public:

	Engine(Config::Section& config) : m_details(config){

	}

	static Engine* Create(std::string engine, Config::Section& details);

	static bool Valid(std::string engine);

	virtual ~Engine(){

	}

	virtual void request(const Hydra::request&, Hydra::reply&) = 0;

protected:

	Config::Section& m_details;

};

}

#endif

