//
// log.cpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "log.hpp"

boost::shared_ptr<Hydra::Log> Hydra::Log::mAccess;
boost::shared_ptr<Hydra::Log> Hydra::Log::mError;

Hydra::Log::Log(std::string name) {

	open(name.c_str(), std::ios_base::out | std::ios_base::app);

}

Hydra::Log::~Log(){

	close();

}

boost::shared_ptr<Hydra::Log> Hydra::Log::access(){

	if(! mAccess)
		mAccess = boost::shared_ptr<Log>(new Log("/var/log/hydra/access.log"));		

	return mAccess;

}

boost::shared_ptr<Hydra::Log> Hydra::Log::error(){

	if(! mError)
		mError = boost::shared_ptr<Log>(new Log("/var/log/hydra/error.log"));		

	return mError;

}

boost::shared_ptr<Hydra::Log> Hydra::Log::custom(std::string name){

	return boost::shared_ptr<Log>(new Log(name));		

}

