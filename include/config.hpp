//
// config.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_CONFIG_HPP
#define HYDRA_FILE_CONFIG_HPP

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

	std::string get(std::string section, std::string label);

	~Config(){

	}

private:

	std::string m_file;
	
	std::map<std::string, std::map<std::string, std::string> > m_data;

};


}


#endif

