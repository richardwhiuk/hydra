//
// main.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <fstream>
#include <string>

#include "server.hpp"

using namespace std;

int main(int argc, char** argv){

	string config = "/etc/hydra.conf";

	if(argc == 2){
		config = argv[1];
	} else if(argc != 1){
		cerr << "Usage: hydra [config]" << endl;
		cerr << endl;
		return 1;
	}

	Hydra::Server hydra(config);
	
	return hydra.go();

}

