//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daemon.hpp"
#include "exception.hpp"
#include "version.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv){

	try {

		if(argc > 1 && std::string(argv[1]) == "-v"){

			std::cout << "Hydra " << HYDRA_VERSION << std::endl;

			return 0;

		}

		Hydra::Daemon h;

		if(argc > 1){
			h.configure(std::string(argv[1]));
		} else {
			h.configure("/etc/hydra.conf");
		}

		h.run();

	} catch (Hydra::Exception* e){

		std::cerr << "Hydra: Exception - " << e->what() << std::endl;

		delete e;

	} catch(std::exception& e){

		std::cerr << e.what() << std::endl;

	}

}

