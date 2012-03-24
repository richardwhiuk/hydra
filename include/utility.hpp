//
// utility.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2012-2012 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>
#include <cstring>

// Only use this if we are going to fork as it's pretty difficult to clear up the resulting corruption.

void strmasscpy(char** dst, size_t place, const std::string src);

