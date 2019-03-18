#include "pistache/endpoint.h"

using namespace Pistache;

class HelloHandler : public Http::Handler {
public:
    HTTP_PROTOTYPE(HelloHandler)
    void onRequest(const Http::Request& request, Http::ResponseWriter response){
        std::cout << "request received" << std::endl;
        response.send(Http::Code::Ok, "Hello World\n");
    }
};

int main() {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(9080));
    auto opts = Http::Endpoint::options().threads(1);
    Http::Endpoint server(addr);
    server.init(opts);
    server.setHandler(std::make_shared<HelloHandler>());
    std::cout << "starting server" << std::endl;
    server.serve();

    std::cout << "shutting down server" << std::endl;
    server.shutdown();
}