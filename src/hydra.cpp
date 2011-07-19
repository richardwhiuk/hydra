#include "hydra.hpp"

#include <iostream>

using namespace std;

bool Hydra::Hydra::run(){

	cout << "Hydra HTTP (" << HYDRA << ") Daemon" << endl;
	cout << "Config File: " << m_config.file() << endl;

	if(!m_config.parse())
		return false;

}

