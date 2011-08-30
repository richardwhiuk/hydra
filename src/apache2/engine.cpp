//
// apache2.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "apache2/engine.hpp"

#include "errno.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

Hydra::Apache2::Engine::Engine(Hydra::Config::Section& config, Hydra::Server& hydra) :
	Hydra::Engine(config, hydra), m_server(*this), m_client(*this)
{
	
}

Hydra::Apache2::Engine::~Engine(){

}

const Hydra::Server& Hydra::Apache2::Engine::server(){
	return m_hydra;
}

const std::string Hydra::Apache2::Engine::name(){
	return m_details.name();
}

const std::vector<std::string> Hydra::Apache2::Engine::user(){
	return m_details["user"];
}

const std::vector<std::string> Hydra::Apache2::Engine::group(){
	return m_details["group"];
}

const std::vector<std::string> Hydra::Apache2::Engine::config(){
	return m_details["config"];
}

bool Hydra::Apache2::Engine::ready(){
	if(!m_server.ready())
		return false;

	if(!m_client.ready())
		return false;

	if(m_details["address"].size() == 0)
		return false;

	if(m_details["port"].size() == 0)
		return false;


	return true;
}

void Hydra::Apache2::Engine::request(Hydra::Connection& con){

	if(!ready()){
		con.reply() = Reply::Stock(Reply::service_unavailable);
		return;
	} 

	m_client.run(con);

}

