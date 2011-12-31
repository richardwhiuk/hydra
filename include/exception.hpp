//
// exception.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_EXCEPTION_HPP
#define HYDRA_FILE_EXCEPTION_HPP

#include <exception>
#include <string>

namespace Hydra {

class Exception : public std::exception {

public:

	Exception(std::string what) throw();
	~Exception() throw();

	virtual const char* what() throw();

private:

	const std::string m_what;

};

}

#endif

