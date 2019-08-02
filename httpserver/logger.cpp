#include "logger.h"
#include <iostream>
#include <sstream>
#include <boost/asio/ssl/error.hpp>

namespace httpserver
{

type_function_logger logger = [](const std::string& msg)
{
    std::clog << msg;
};

void fail(boost::system::error_code ec, const char *what)
{
    // ssl::error::stream_truncated, also known as an SSL "short read",
    // indicates the peer closed the connection without performing the
    // required closing handshake (for example, Google does this to
    // improve performance). Generally this can be a security issue,
    // but if your communication protocol is self-terminated (as
    // it is with both HTTP and WebSocket) then you may simply
    // ignore the lack of close_notify.
    //
    // https://github.com/boostorg/beast/issues/38
    //
    // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
    //
    // When a short read would cut off the end of an HTTP message,
    // Beast returns the error beast::http::error::partial_message.
    // Therefore, if we see a short read here, it has occurred
    // after the message has been completed, so it is safe to ignore it.

    if(ec == boost::asio::ssl::error::stream_truncated)
        return;

    std::stringstream ss; ss << what << ": " << ec.message() << "\n";
    logger( ss.str() );
}

}
