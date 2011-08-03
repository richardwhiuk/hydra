//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_HEADER_HPP
#define HYDRA_FILE_HEADER_HPP

#include <string>

namespace Hydra {

struct Header
{
  std::string name;
  std::string value;
};

}

#endif
