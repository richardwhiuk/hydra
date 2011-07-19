#ifndef HYDRA_FILE_CONFIG
#define HYDRA_FILE_CONFIG

#include <string>
#include <map>

namespace Hydra {

class Config {

public:

	enum ParseMode {
		START,
		SECTION_FIRST,
		SECTION,
		LABEL,
		LABEL_WS,
		SEPARATOR,
		VALUE,
		VALUE_WS,
		END,
		COMMENT,
		ERROR,
	};

	Config(std::string& file){
		m_file = file;
	}

	const std::string& file(){
		return m_file;
	}

	bool parse();

	~Config(){

	}

private:

	std::string m_file;
	
	std::map<std::string, std::map<std::string, std::string> > m_data;

};


}


#endif

