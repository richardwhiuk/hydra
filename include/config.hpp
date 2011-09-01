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
#include <vector>

namespace Hydra {

class Config {

public:

	class Section : public std::map<std::string, std::vector<std::string> > {

	public:
		Section(std::string);
		~Section();
		const std::string& name();

	private:

		std::string m_name;

	};

	typedef std::vector<Section> Data;
	typedef Data::iterator Iterator;

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

	Config(std::string& file);

	virtual ~Config();

	const std::string& file();

	bool parse();

	Config::Iterator begin();

	Config::Iterator end();

	std::string get(std::string section, std::string label);

	Config::Iterator get(std::string section);

private:

	std::string m_file;
	Config::Data m_data;

};


}


#endif

