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
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "header.hpp"

namespace Hydra {

/// A reply to be sent to a client.
class Reply
{
	/// Reply Status Codes.

public:

	enum reply_state_out
	{
		out_none,
		out_status,
		out_header,
		out_content,
		out_done
	};

	enum reply_state_in
	{
		in_none,
		in_status,
		in_header,
		in_content_none,
		in_content_data,
		in_done
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

	Reply();
	~Reply();

	// Used by the Engine to set the reply.

	void status(status_type status);

	void header(Hydra::Header& header);

	void headers_complete();

	void content();				// Signal content can be started
	void content(std::string);		// Give content to reply with.

	void content_bind(boost::function<void()>);

	void finish();

	/// Get a stock reply.

	void stock(status_type status);

	// Used by the Connection to send the response

	// Get next set of available buffers.

	bool buffer(std::vector<boost::asio::const_buffer>& buffer);

	/// Last buffers have been sent and underlying data may now be discarded.

	void discard();

private:

	/// Reply status

	status_type m_status;

	// State of incoming and outgoing streams.

	reply_state_in m_in_state;
	reply_state_out m_out_state;

	// Condition sync

	boost::condition_variable m_in_cond;
	boost::mutex m_in_mutex;			// In/out transition mutex.

	// We need more content.

	boost::function<void()> m_in_bind;
	bool m_in_bind_called;

	/// The headers to be included in the reply.
	std::vector<Hydra::Header> m_headers;

	/// The content to be sent in the reply.
	
	std::queue<std::string*> m_in;

	// The content being sent in the reply.

	std::vector<std::string*> m_out;

};

}

#endif 
