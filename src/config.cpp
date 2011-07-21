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

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Hydra::Config::Section::Section(std::string name) : m_name(name){

}

Hydra::Config::Section::~Section(){

}

std::string Hydra::Config::Section::name(){
	return m_name;
}


Hydra::Config::Config(std::string& file){
	m_file = file;
}

Hydra::Config::~Config(){

}

const std::string& Hydra::Config::file(){
	return m_file;
}

Hydra::Config::Iterator Hydra::Config::get(std::string section){
	for(Hydra::Config::Iterator it = m_data.begin(); it != m_data.end(); ++it){
		if(it->name() == section){
			return it;
		}
	}
	return m_data.end();
}

std::string Hydra::Config::get(std::string section, std::string label){
	Hydra::Config::Iterator it = get(section);
	return (*it)[label];
}

Hydra::Config::Iterator Hydra::Config::begin(){
	return m_data.begin();
}

Hydra::Config::Iterator Hydra::Config::end(){
	return m_data.end();
}

bool Hydra::Config::parse(){

	string line;

	ifstream in(m_file.c_str(), ifstream::in);

	if(! in.is_open()){
		std::cerr << "Config File Error: Unable to open configuration file." << std::endl;
		return false;
	}

	getline(in, line);

	Section section("");
	string label = "";
	string ws;
	string value;
	string error;
	int line_no = 0;

	while(in.good() && !in.eof()){
		
		line_no ++;
		ParseMode lineMode = START;

		for(size_t i = 0; i < line.length() && lineMode != ERROR && lineMode != END; i++){
			if(lineMode == START){
				if(line[i] == '['){
					lineMode = SECTION_FIRST;
				} else if(line[i] == '#'){ 
					lineMode = COMMENT;
				} else if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					// skip
				} else if(line[i] == ':') {
					error = "Blank Label";
					lineMode = ERROR;
				} else {
					label = line[i];
					lineMode = LABEL;
				}
			} else if(lineMode == SECTION_FIRST) {
				if(line[i] == ']'){
					lineMode = ERROR;
					error = "Blank section name";
				} else if(line[i] == '#'){
					lineMode = ERROR;
					error = "Comment in section name";
				} else {
					label = line[i];
					lineMode = SECTION;
				}
			} else if(lineMode == SECTION) {
				if(line[i] == ']'){
					m_data.push_back(section);
					section = Section(label);
					lineMode = START;
				} else if(line[i] == '#'){
					lineMode = ERROR;
					error = "Comment in section name";
				} else {
					label.push_back(line[i]);
				}
			} else if(lineMode == LABEL) {
				if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					ws = line[i];
					lineMode = LABEL_WS;
				} else if(line[i] == '#'){
					lineMode = ERROR;
					error = "Comment in label";
				} else if(line[i] == ':') {
					lineMode = SEPARATOR;
				} else {
					label.push_back(line[i]);
				}
			} else if(lineMode == LABEL_WS) {
				if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					ws.push_back(line[i]);
				} else if(line[i] == '#'){
					lineMode = ERROR;
					error = "Comment in label";
				} else if(line[i] == ':') {
					lineMode = SEPARATOR;
				} else {
					label.append(ws);
					label.push_back(line[i]);
					lineMode = LABEL;
				}
			} else if(lineMode == SEPARATOR) {
				if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					// Ignore
				} else if(line[i] == '#'){
					value = "";
					lineMode = END;
				} else {
					value = line[i];
					lineMode = VALUE;
				}
			} else if(lineMode == VALUE){
				if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					ws = line[i];
					lineMode = VALUE_WS;
				} else if(line[i] == '#'){
					lineMode = END;
				} else {
					value += line[i];
				}
			} else if(lineMode == VALUE_WS){
				if(line[i] == '\t' || line[i] == ' ' || line[i] == '\r' || line[i] == '\n') {
					ws += line[i];
				} else if(line[i] == '#'){
					lineMode = END;
				} else {
					value += ws;
					value += line[i];
					lineMode = VALUE;
				}
			}
		}

		if(lineMode == START){
			// Empty line or section heading.
		} else if (lineMode == ERROR){
			// Error in line.
			std:cerr << "Config File Error: Line: " << line_no << " Error: " << error << '.'  << endl;
			return false;
		} else if (lineMode == LABEL || lineMode == LABEL_WS || lineMode == SEPARATOR || lineMode == SECTION_FIRST || lineMode == SECTION){
			// Invalid syntax
			cerr << "Config File Error: Line: " << line_no << " Invalid Syntax." << endl;
			return false;
		} else if (lineMode == VALUE_WS || lineMode == VALUE || lineMode == END){
			// Valid line
			section[label] = value;
		}

		getline(in, line);
	}
	m_data.push_back(section);

	return true;

}

