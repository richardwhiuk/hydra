#ifndef HYDRA_FILE_HYDRA
#define HYDRA_FILE_HYDRA

#include <string>

#include "config.hpp"
#include "version.hpp"

namespace Hydra {

class Hydra {

public:

	Hydra(std::string& config) : m_config(config){
	
	}

	bool run();

	~Hydra(){

	}

private:

	::Hydra::Config m_config;


};

}

#endif

