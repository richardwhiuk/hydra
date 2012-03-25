//
// config.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "exception.hpp"

#include <iostream>
#include <fstream>

Hydra::Config::Config(){

}

Hydra::Config::~Config(){

}

void Hydra::Config::open(std::string filename){

// Parse file.

	parse(filename);

	m_filename = filename;

}

void Hydra::Config::refresh(){

// Reparse file.

	parse(m_filename);

}

// Parser:

enum LineType {
	NONE,
	SECTION,
	VALUE
};

enum ParserMode {
	LBRACE_KEY,
	SECTION_CHAR,
	SECTION_CHAR_RBRACE,
	SECTION_END,
	KEY_CHAR,
	TAG_CHAR,
	VALUE_CHAR_START,
	VALUE_CHAR,
	COMMENT
};

void Hydra::Config::parse(const std::string& filename){

// File format:
//
// [section]
//
// key = value # Comment
//
// # comment 

	std::string section = "global";

	std::ifstream file;
	file.open(filename.c_str());

	if(!file.is_open()){
		throw new Hydra::Exception(std::string("Hydra->Config->Failed to open file: ").append(filename));
	}

	std::map<std::string, Section> data;

	while(file.good()){

		LineType type = NONE;
		ParserMode mode = LBRACE_KEY;

		std::string line;
	
		getline(file, line);

		size_t a,b,c,d,e,f;

		for(size_t i = 0; i < line.length() && mode != COMMENT; ++i){

			char& x = line[i];

			if(mode == LBRACE_KEY){
				if(x == '['){
					type = SECTION;
					mode = SECTION_CHAR;
					a = i + 1;
				} else if(x == '\r' || x == '\n' || x == '\t' || x == ' ') {
					// Skip
				} else if(x == '#'){
					mode = COMMENT;
				} else if(x == '=') {
					throw new Hydra::Exception(std::string("Hydra->Config->Blank label in configuration file: ").append(filename));
				} else {
					type = VALUE;
					mode = KEY_CHAR;
					a = i;
					e = f = b = i + 1;
				}
			} else if(mode == SECTION_CHAR) {
				if( x == ']'){
					throw new Hydra::Exception(std::string("Hydra->Config->Blank section label in configuration file: ").append(filename));
				} else if(x == '\r' || x == '\n' || x == '\t' || x == ' ') {
					throw new Hydra::Exception(std::string("Hydra->Config->Invalid character in section label in configuration file: ").append(filename));
				} else {
					mode = SECTION_CHAR_RBRACE;
				}
			} else if(mode == SECTION_CHAR_RBRACE) {
                                if( x == ']'){
					mode = SECTION_END;
					b = i - 1;
                                } else if(x == '\r' || x == '\n' || x == '\t' || x == ' ') {
                                        throw new Hydra::Exception(std::string("Hydra->Config->Invalid character in section label in configuration file: ").append(filename));
                                } else {
					// Valid character
                                }
                        } else if(mode == SECTION_END) {
				if(x == '\r' || x == '\n' || x == '\t' || x == ' ') {
                                        // Skip
                                } else if(x == '#'){
					mode = COMMENT;
				} else {
                                        throw new Hydra::Exception(std::string("Hydra->Config->Invalid character in section label in configuration file: ").append(filename));
                                }
			} else if(mode == KEY_CHAR){
				if(x == '='){
					mode = VALUE_CHAR_START;
				} else if(x == ':'){
					mode = TAG_CHAR;
					e = f = i + 1;
				} else if(x == '\r' || x == '\n' || x == '\t' || x == ' '){
					// Skip
					// will be included if we have more text - this allows us to have 'a b' = 'c d'
				} else {
					e = f = b = i + 1;
				}
			} else if(mode == TAG_CHAR){
				if(x == '='){
					mode = VALUE_CHAR_START;
				} else if(x == '\r' || x == '\n' || x == '\t' || x == ' '){
					// Skip
				} else {
					f = i + 1;
				}
			} else if(mode == VALUE_CHAR_START){
				if(x == '\r' || x == '\n' || x == ' ' || x == '\t'){
					// Skip
				} else if(x == '#'){
					throw new Hydra::Exception(std::string("Hydra->Config->Blank value in configuration file: ").append(filename));
				} else {
					c = i;
					d = i + 1;
					mode = VALUE_CHAR;
				}
			} else if(mode == VALUE_CHAR){
				if(x == '\r' || x == '\n' || x == ' ' || x == '\t'){
					// Skip
				} else if(x == '#'){
					mode = COMMENT;
				} else {
					d = i + 1;
				} 
			}

		}


		if(type == SECTION && ( mode == SECTION_END || mode == COMMENT ) ){
			// Valid section

			section = line.substr(a,b - a);

		} else if(type == VALUE && ( mode == COMMENT || mode == VALUE_CHAR ) ){
			// Valid key value pair

			data[section].value(line.substr(a,b - a),line.substr(c,d - c), line.substr(e, f -e));

		} else if(type == NONE){
			// Valid open line.

		} else {
			
			throw new Hydra::Exception(std::string("Hydra->Config->Invalid line configuration file: ").append(filename));

		}

	}

	if(!file.eof() || file.bad()){
		throw new Hydra::Exception(std::string("Hydra->Config->Failed to read configuration file: ").append(filename));
	}

	m_sections = data;

}

Hydra::Config::Section Hydra::Config::section(std::string name){

// Take out a lock

	std::map<std::string, Hydra::Config::Section>::const_iterator section = m_sections.find(name);

	if(section == m_sections.end()){

		throw new Hydra::Exception(std::string("Hydra->Config->Section not found: ").append(name));

	}

// Return the lock

	return section->second;

}

std::map<std::string, Hydra::Config::Section> Hydra::Config::sections(){

	return m_sections;

}

Hydra::Config::Section::Section(){

}

Hydra::Config::Section::~Section(){

}

void Hydra::Config::Section::value(std::string key, std::string value, std::string tag){
	// Set key => value

	std::list<std::string>& values = m_values[key][tag];

	values.push_front(value);
}

std::string Hydra::Config::Section::value(std::string key){
	// Return single value	

	std::list<std::string>& values = m_values[key][""];

	if(values.empty()){

		throw new Hydra::Exception(std::string("Hydra->Config->Section->Key not found: ").append(key));

	}

	return values.front();

}

std::list<std::string> Hydra::Config::Section::values(const std::string key){
	// Return [key]
	
	return m_values[key][""];

}

std::string Hydra::Config::Section::value_tag(const std::string& key, const std::string& tag){

	std::list<std::string>& values = m_values[key][tag];

	if(values.empty()){

		return value(key);

	}

	return values.front();

}

std::list<std::string> Hydra::Config::Section::values_tag(const std::string& key, const std::string& tag){

	return m_values[key][tag];

}

std::ostream& Hydra::operator<< (std::ostream& o, Hydra::Config const& config){

	for(std::map<std::string, Hydra::Config::Section>::const_iterator it = config.m_sections.begin(); it != config.m_sections.end(); ++it){
		o << std::endl << it->first << " => " << it->second;
	}

	return o; 
}

std::ostream& Hydra::operator<< (std::ostream& o, Hydra::Config::Section const& section){
	
	for(std::map<std::string, std::map<std::string, std::list<std::string> > >::const_iterator it = section.m_values.begin(); it != section.m_values.end(); ++it){
		o << std::endl << '\t' << it->first << " => ";
		for(std::map<std::string, std::list<std::string> >::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit){
			o << std::endl << "\t\t" << sit->first << " => ";
			for(std::list<std::string>::const_iterator tit = sit->second.begin(); tit != sit->second.end(); ++sit){
				o << std::endl << "\t\t\t" << *tit;
			}
		}
	}

	return o;
}

