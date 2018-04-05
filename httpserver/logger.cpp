#include "logger.h"
#include <iostream>
#include <sstream>

namespace httpserver
{

type_function_logger logger = [](const std::string& msg)
{
    std::clog << msg;
};

void fail(boost::system::error_code ec, const char *what)
{
    std::stringstream ss; ss << what << ": " << ec.message() << "\n";
    logger( ss.str() );
}

}
