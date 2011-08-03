//
// status_strings.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "status_strings.hpp"

boost::asio::const_buffer Hydra::status_strings::to_buffer(Reply::status_type status){
	switch (status)
	{
	case Reply::ok:
		return boost::asio::buffer(ok);
	case Reply::created:
		return boost::asio::buffer(created);
	case Reply::accepted:
		return boost::asio::buffer(accepted);
	case Reply::no_content:
		return boost::asio::buffer(no_content);
	case Reply::multiple_choices:
		return boost::asio::buffer(multiple_choices);
	case Reply::moved_permanently:
		return boost::asio::buffer(moved_permanently);
	case Reply::moved_temporarily:
		return boost::asio::buffer(moved_temporarily);
	case Reply::not_modified:
		return boost::asio::buffer(not_modified);
	case Reply::bad_request:
		return boost::asio::buffer(bad_request);
	case Reply::unauthorized:
		return boost::asio::buffer(unauthorized);
	case Reply::forbidden:
		return boost::asio::buffer(forbidden);
	case Reply::not_found:
		return boost::asio::buffer(not_found);
	case Reply::internal_server_error:
		return boost::asio::buffer(internal_server_error);
	case Reply::not_implemented:
		return boost::asio::buffer(not_implemented);
	case Reply::bad_gateway:
		return boost::asio::buffer(bad_gateway);
	case Reply::service_unavailable:
		return boost::asio::buffer(service_unavailable);
	default:
		return boost::asio::buffer(internal_server_error);
	}
}


