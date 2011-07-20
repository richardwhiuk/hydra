//
// io_service_pool.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2011-2011 Richard J. Whitehouse (richard at richardwhiuk dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HYDRA_FILE_IO_SERVICE_POOL_HPP
#define HYDRA_FILE_IO_SERVICE_POOL_HPP

#include <boost/asio.hpp>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace Hydra {

/// A pool of io_service objects.
class io_service_pool : public boost::enable_shared_from_this<io_service_pool>, private boost::noncopyable{

public:
	/// Construct the io_service pool.
	explicit io_service_pool(std::size_t pool_size);

	/// Run all io_service objects in the pool.
	void run();

	/// Stop all io_service objects in the pool.
	void stop();

	/// Get an io_service to use.
	boost::asio::io_service& get_io_service();

private:
	typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
	typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;

	// Size of pool
	std::size_t m_size;

	/// The pool of io_services.
	std::vector<io_service_ptr> m_services;

	/// The work that keeps the io_services running.
	std::vector<work_ptr> m_work;

	/// The next io_service to use for a connection.
	std::size_t m_next;
};

typedef boost::shared_ptr<io_service_pool> io_service_pool_ptr;

}

#endif // HTTP_SERVER2_IO_SERVICE_POOL_HPP
