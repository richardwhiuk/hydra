//
// exception.cpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "exception.hpp"

Hydra::Exception::Exception(std::string what) throw() : m_what(what){

}

Hydra::Exception::~Exception() throw(){

}

const char* Hydra::Exception::what() throw(){

	return m_what.c_str();

}

