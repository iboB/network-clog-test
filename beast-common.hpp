#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW 1
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT 1

#if defined(_MSC_VER)
#   pragma warning (disable: 4100)
#endif
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

