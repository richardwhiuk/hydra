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
#include <list>

namespace Hydra {

class Config {

public:

	friend std::ostream& operator<< (std::ostream& o, Hydra::Config const& config);

	class Section {

	public:

		friend std::ostream& operator<< (std::ostream& o, Hydra::Config::Section const& section);

		Section();
		~Section();

		void value(const std::string, const std::string);
		std::string value(const std::string);
		std::list<std::string> values(const std::string);

		std::string value_tag(const std::string&, const std::string&);
		std::list<std::string> values_tag(const std::string&, const std::string&);

	private:

		std::map<std::string, std::list<std::string> > m_values;

	};

	Config();
	~Config();

	void open(std::string filename);
	void refresh();

	Section section(std::string name);
	std::map<std::string, Section> sections();

private:

	void parse(const std::string& filename);

	std::string m_filename;
	std::map<std::string, Section> m_sections;

};

std::ostream& operator<< (std::ostream& o, Hydra::Config const& config);
std::ostream& operator<< (std::ostream& o, Hydra::Config::Section const& section);

}

#endif

