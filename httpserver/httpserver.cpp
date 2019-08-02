#include "httpserver.h"
#include "session_plain.h"
#include "session_ssl.h"
#include "server_certificate.hpp"

namespace httpserver {

void HttpServer::run(const string &addr, std::vector<std::pair<unsigned short, bool>> ports, const string &doc_root, int thread_qtd, const std::string& certificadoFileName, const std::string& chaveFileName)
{
    router_.set_public_dir(doc_root);

    auto const address = net::ip::make_address(addr);
    auto const threads = std::max<int>(1, thread_qtd);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx, certificadoFileName, chaveFileName);

    // Create and launch a listening port
    for(auto& port: ports){
        if(port.second)
            std::make_shared<listener_ssl>(
                        ioc,
                        ctx,
                        tcp::endpoint{address, port.first},
                        router_)->run();
        else
            std::make_shared<listener_palin>(
                        ioc,
                        tcp::endpoint{address, port.first},
                        router_)->run();

    }

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                    [&ioc]
        {
            ioc.run();
        });
    ioc.run();
}

}
