#ifndef LOGGER_H
#define LOGGER_H

#include <functional>
#include <boost/system/error_code.hpp>

namespace httpserver {

typedef std::function<void(const std::string&)> type_function_logger;

extern type_function_logger logger;

// Report a failure
void fail(boost::system::error_code ec, const char *what);

}

#endif // LOGGER_H

