//
// log.hpp
// ~~~~~~~
//
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <fstream>
#include <string>

#include <boost/shared_ptr.hpp>

namespace Hydra {

class Log : public std::ofstream {

public:

	virtual ~Log();

	static boost::shared_ptr<Log> access();
	static boost::shared_ptr<Log> error();
	static boost::shared_ptr<Log> custom(std::string name);

private:

	std::ofstream mLog;

	Log(std::string name);

	static boost::shared_ptr<Log> mAccess;
	static boost::shared_ptr<Log> mError;

};

}

