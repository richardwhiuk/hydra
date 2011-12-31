//
// ssl.cpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "client/ssl.hpp"

Hydra::Client::SSL::SSL(std::string name, Config::Section config, Daemon& hydra) : Base(name, config, hydra){

}

Hydra::Client::SSL::~SSL(){

}

void Hydra::Client::SSL::run(boost::asio::io_service&){

}

