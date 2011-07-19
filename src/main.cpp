#include <iostream>
#include <fstream>
#include <string>

#include "hydra.hpp"

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

	Hydra::Hydra hydra(config);

	hydra.run();

	return 0;

}

