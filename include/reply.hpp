//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_REPLY_HPP
#define HYDRA_FILE_REPLY_HPP

#include <string>
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include "header.hpp"

namespace Hydra {

/// A reply to be sent to a client.
class Reply
{
	/// Reply Status Codes.

public:

	Reply();
	Reply& operator=(const Reply&);
	Reply(const Reply&);
	~Reply();

	enum reply_state
	{
		none,
		header,
		partial,
		done
	};

	enum status_type
	{
		ok = 200,
		created = 201,
		accepted = 202,
		no_content = 204,
		multiple_choices = 300,
		moved_permanently = 301,
		moved_temporarily = 302,
		not_modified = 304,
		bad_request = 400,
		unauthorized = 401,
		forbidden = 403,
		not_found = 404,
		internal_server_error = 500,
		not_implemented = 501,
		bad_gateway = 502,
		service_unavailable = 503
	};

	/// Reply status

	status_type status;

	reply_state state;

	/// The headers to be included in the reply.
	std::vector<Hydra::Header> headers;

	/// The content to be sent in the reply.
	
	std::queue<std::string*> tosend;

	std::vector<std::string*> sending;

	boost::mutex m_tosend_mux;
	boost::mutex m_sending_mux;

	void content(std::string);

	/// Convert the reply into a vector of buffers. The buffers do not own the
	/// underlying memory blocks, therefore the reply object must remain valid and
	/// not be changed until the write operation has completed.

	std::vector<boost::asio::const_buffer> buffers();

	/// Last buffers have been sent and underlying data may now be discarded

	void discard();

	/// Get a stock reply.
	static Reply Stock(status_type status);
};

}

#endif 
