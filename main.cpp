//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, synchronous
//
//------------------------------------------------------------------------------

#include <rtc/rtc.hpp>
#include <boost/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <future>
#include <string>
#include <thread>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------
std::unique_ptr<rtc::PeerConnection> pc;

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req)
{
  using namespace boost::json;
  std::string msg;
  try
  {
    value remote = parse(req.body());
    rtc::Configuration config;
    config.enableIceUdpMux = true;
    config.portRangeBegin = 8081;
    config.portRangeEnd = 8082;

    pc = std::make_unique<rtc::PeerConnection>(config);
    pc->onDataChannel([](std::shared_ptr<rtc::DataChannel> dc)
                      {
dc->onOpen([dc](){
  std::cout << dc->label() << " datachannel opened\n";
});
dc->onMessage([dc](rtc::binary data){
  
   std::string dcMsg;
  for( auto it = data.begin(); it != data.end(); ++it ) {
        dcMsg.push_back( (char)*it );
    }
std::cout << dc->label() << " datachannel message = "<< dcMsg << std::endl;
}, [dc](std::string message){
std::cout << dc->label() << " datachannel string message = " << message << std::endl;
}); });
    std::promise<bool> promise;
    auto future = promise.get_future();
    pc->onLocalDescription([&promise](rtc::Description localDescription)
                           {
      std::cout << "local description ready";
      promise.set_value(true); });
    std::promise<bool> completeGathering;
    auto gathering = completeGathering.get_future();
    pc->onGatheringStateChange([&completeGathering](rtc::PeerConnection::GatheringState state)
                               {
      std::cout << "gathering "<< (int)state <<std::endl;
      if (state == rtc::PeerConnection::GatheringState::Complete){
        completeGathering.set_value(true);
      } });
    std::string remoteSdp = remote.as_object()["sdp"].as_string().c_str();
    std::string remoteType = remote.as_object()["type"].as_string().c_str();
    rtc::Description remoteDescription(remoteSdp, remoteType);
    pc->setRemoteDescription(remoteDescription);
    future.get();

    gathering.get();
    auto localDescription = pc->localDescription().value();

    object answer;
    answer["sdp"] = (std::string)localDescription;
    answer["type"] = localDescription.typeString();
    msg = serialize(answer);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
  // Returns a bad request response
  auto const bad_request =
      [&req](beast::string_view why)
  {
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Returns a not found response
  auto const not_found =
      [&req](beast::string_view target)
  {
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  };

  // Returns a server error response
  auto const server_error =
      [&req](beast::string_view what)
  {
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if (req.method() != http::verb::post)
    return bad_request("Unknown HTTP-method");

  // Request path must be absolute and not contain "..".
  if (req.target().empty() ||
      req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");

  // Respond to POST request

  http::response<http::string_body> res{http::status::ok, req.version()};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "application/json");
  res.set(http::field::access_control_allow_origin, "*");
  res.body().swap(msg);
  res.prepare_payload();
  res.keep_alive(req.keep_alive());
  return res;
}

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what)
{
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
void do_session(
    tcp::socket &socket)
{
  beast::error_code ec;

  // This buffer is required to persist across reads
  beast::flat_buffer buffer;

  for (;;)
  {
    // Read a request
    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);
    if (ec == http::error::end_of_stream)
      break;
    if (ec)
      return fail(ec, "read");

    // Handle request
    http::message_generator msg =
        handle_request(std::move(req));

    // Determine if we should close the connection
    bool keep_alive = msg.keep_alive();

    // Send the response
    beast::write(socket, std::move(msg), ec);

    if (ec)
      return fail(ec, "write");
    if (!keep_alive)
    {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      break;
    }
  }

  // Send a TCP shutdown
  socket.shutdown(tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully
}

int main(int argc, char *argv[])
{
  rtcPreload();
  rtc::InitLogger(rtc::LogLevel::Info);

  try
  {
    // Check command line arguments.
    if (argc != 2)
    {
      std::cerr << "Usage: http-server-sync <port>\n"
                << "Example:\n"
                << "    http-server-sync 8080 \n";
      return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi(argv[1]));

    // The io_context is required for all I/O
    net::io_context ioc{1};

    // The acceptor receives incoming connections
    tcp::acceptor acceptor{ioc, {address, port}};
    for (;;)
    {
      // This will receive the new connection
      tcp::socket socket{ioc};

      // Block until we get a connection
      acceptor.accept(socket);

      // Launch the session, transferring ownership of the socket
      std::thread{std::bind(
                      &do_session,
                      std::move(socket))}
          .detach();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  rtcCleanup();

  return 0;
}
