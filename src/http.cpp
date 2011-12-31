//
// http.cpp
// ~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "http.hpp"

bool Hydra::HTTP::is_ctl(char c){
	if(c >= 0 && c <= 31){
		return true;
	} else if(c == 127){
		return true;
	} else {
		return false;
	}
}

bool Hydra::HTTP::is_char(char c){
	if(c >= 0 && c <= 127){
		return true;
	}
	return false;
}

bool Hydra::HTTP::is_alpha(char c){
	if(c >= 'a' && c <= 'z'){
		return true;
	} else if(c >= 'A' && c <= 'Z'){
		return true;
	} else {
		return false;
	}
}

bool Hydra::HTTP::is_special(char c){
	switch(c){
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
	}
	return false;
}

bool Hydra::HTTP::is_digit(char c){

	if(c >= '0' && c <= '9'){
		return true;
	}

	return false;

}

