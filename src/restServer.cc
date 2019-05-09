#include <algorithm>
#include <iostream>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "ProgOpts.h"
#include "spdlog/spdlog.h"
#include "mstQuery.h"

// using namespace std;
using namespace Pistache;

int mst_query_main(QueryOpts &opt);
int build_mst_main (QueryOpts &opt);
int build_main(BuildOpts &opt);
auto console = spdlog::stdout_color_mt("mantis_console");

void printCookies(const Http::Request& req){
    auto cookies = req.cookies();
    std::cout << "cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies){
        std::cout << indent << c.name << " = "<< c.value << std::endl;
    }
    std::cout << "]" <<std::endl;
}

class MantisEndpoint{
public:
    MantisEndpoint(Address addr)
    : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    {}

    void init(size_t thr = 1){
        auto opts = Http::Endpoint::options()
                .threads(thr)
                .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start(){
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown(){
        httpEndpoint->shutdown();
    }
private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/auth", Routes::bind(&MantisEndpoint::doAuth, this));
        Routes::Post(router, "/query/:p/:o/:i", Routes::bind(&MantisEndpoint::doQuery, this));
        Routes::Post(router, "/build-mst/:p/:t/:k", Routes::bind(&MantisEndpoint::doBuildMst, this));
        Routes::Post(router, "/build/:s/:i/:o", Routes::bind(&MantisEndpoint::doBuild, this));
    }

    std::string urlDecode(std::string SRC) {
        std::string ret;
        char ch;
        int i, ii;
        for (i=0; i<SRC.length(); i++) {
            if (int(SRC[i])==37) {
                sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
                ch=static_cast<char>(ii);
                ret+=ch;
                i=i+2;
            } else {
                ret+=SRC[i];
            }
        }
        return (ret);
    }

    void doQuery(const Rest::Request& request, Http::ResponseWriter response) {
        if (!request.hasParam(":p")) {
            response.send(Http::Code::Bad_Request, "Query Prefix is required\n");
        }
        auto query_prefix = request.param(":p");
        std::string qp = query_prefix.as<std::string>();
        qp = urlDecode(qp);

        if (!request.hasParam(":o")) {
            response.send(Http::Code::Bad_Request, "Output file is required\n");
        }
        auto output = request.param(":o");
        std::string op = output.as<std::string>();
        op = urlDecode(op);

        if (!request.hasParam(":i")) {
            response.send(Http::Code::Bad_Request, "Input file is required\n");
        }
        auto input = request.param(":i");
        std::string ip = input.as<std::string>();
        ip = urlDecode(ip);

        std::cout << qp << " " << op << " " << ip << std::endl;

        QueryOpts qopt;
        qopt.console = console;
        qopt.prefix = qp;
        qopt.output = op;
        qopt.query_file = ip;
        int rp = mst_query_main(qopt);
        std::cout << rp << std::endl;
        response.send(Http::Code::Ok, "parameters received\n");
    }

    void doBuildMst(const Rest::Request& request, Http::ResponseWriter response) {
        if (!request.hasParam(":p")) {
            response.send(Http::Code::Bad_Request, "The path to directory where index is stored is required\n");
        }
        auto query_prefix = request.param(":p");
        std::string qp = query_prefix.as<std::string>();
        qp = urlDecode(qp);

        int t = 1;
        if (request.hasParam(":t")) {
            auto threads = request.param(":t");
            t = threads.as<int>();
            //t = urlDecode(t);
        }

        int k = 1;
        if (request.hasParam(":k")) {
            auto keep = request.param(":k");
            k = keep.as<int>();
            //t = urlDecode(t);
        }

        std::cout << qp << " " << t << " " << k << std::endl;

        QueryOpts qopt;
        qopt.console = console;
        qopt.prefix = qp;
        qopt.keep_colorclasses = 1;
        qopt.numThreads = t;
        int rp = build_mst_main(qopt);
        std::cout << rp << std::endl;
        response.send(Http::Code::Ok, "parameters received\n");
    }

    void doBuild(const Rest::Request& request, Http::ResponseWriter response) {
        // if (!request.hasParam(":s")) {
        //     response.send(Http::Code::Bad_Request, "The path to directory where index is stored is required\n");
        // }
        // auto query_prefix = request.param(":p");
        // std::string qp = query_prefix.as<std::string>();
        // qp = urlDecode(qp);
        int s = 1;
        if (request.hasParam(":s")) {
            auto squeakr = request.param(":s");
            s = squeakr.as<int>();
        }
        
        if (!request.hasParam(":i")) {
            response.send(Http::Code::Bad_Request, "Input file is required\n");
        }
        auto input = request.param(":i");
        std::string ip = input.as<std::string>();
        ip = urlDecode(ip);

        if (!request.hasParam(":o")) {
            response.send(Http::Code::Bad_Request, "Output file is required\n");
        }
        auto output = request.param(":o");
        std::string op = output.as<std::string>();
        op = urlDecode(op);

        std::cout << s << " " << ip << " " << op << std::endl;

        BuildOpts bopt;
        bopt.console = console;
        bopt.qbits = s;
        bopt.out = op;
        bopt.inlist = ip;
        int rp = build_main(bopt);
        std::cout << rp << std::endl;
        response.send(Http::Code::Ok, "parameters received\n");
    }

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok, "OK\n");
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << thr << " threads" << std::endl;

    MantisEndpoint stats(addr);

    stats.init(thr);
    stats.start();

    stats.shutdown();
}