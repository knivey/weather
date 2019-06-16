#include "FileReader.hpp"
#include "location.h"
#include "weather.h"

#include <fmt/core.h>

#include "grpc/ultimateq.grpc.pb.h"
#include "grpc/ultimateq.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <boost/algorithm/string.hpp>

#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace api;
using namespace std::literals;

constexpr std::string_view EXTNAME = "weather";
constexpr std::string_view NETNAME = "zkpq";

static std::string DARKSKY_KEY;
static std::string BING_KEY;

class ExtClient {
public:
    ExtClient(std::shared_ptr<Channel> channel) : stub(Ext::NewStub(channel)) {
        ;
    }
    std::unique_ptr<Ext::Stub> stub;

    void registerPubCmd(std::string name, std::string desc, std::vector<std::string> args) {
        cmds_.emplace_back(std::make_unique<Cmd>());

        auto cmd = cmds_.back().get();
        cmd->set_ext(EXTNAME.data());
        cmd->set_name(name);
        cmd->set_desc(desc);
        cmd->set_kind(Cmd::AnyKind);
        cmd->set_scope(Cmd::Public);
        std::for_each(args.begin(), args.end(),
            [&cmd] (const auto& arg) {
                *cmd->add_args() = arg;
            }
        );

        RegisterCmdRequest cmd_request;
        cmd_request.set_ext(EXTNAME.data());
        cmd_request.set_allocated_cmd(cmd);

        ClientContext context;
        RegisterResponse RegResp;
        if (auto status = stub->RegisterCmd(&context, cmd_request, &RegResp); !status.ok()) {
            std::cout << "RegisterCmd rpc failed." << std::endl;
            std::cout << status.error_code() << std::endl;
            std::cout << status.error_message() << std::endl;
        }
    }

    void registerCmds() {
        registerPubCmd("wz", "check the weather", {"location..."});
        registerPubCmd("weather", "check the weather", {"location..."});
        registerPubCmd("snek", "hssss", {});
    }

    void putchan(std::string_view chan, std::string msg) {
        std::string rawmsg = fmt::format("PRIVMSG {} :{}", chan, msg);
        std::cout << "<< " << rawmsg << std::endl;
        WriteRequest wr;
        wr.set_ext(EXTNAME.data());
        wr.set_net(NETNAME.data());
        wr.set_msg(rawmsg);
        ClientContext context;
        Empty resp;
        if (auto status = stub->Write(&context, wr, &resp); !status.ok()) {
            std::cout << "Write rpc failed." << std::endl;
            std::cout << status.error_code() << std::endl;
            std::cout << status.error_message() << std::endl;
        }
    }
    
    std::string GetUnits(std::string &q) {
        std::string units = "auto";
        std::string out = "";
        std::vector<std::string> words;
        boost::split(words, q, boost::is_space());
        for(std::string word : words) {
            if (word.substr(0, 2) == "--") {
                std::string unit = word.substr(2, std::string::npos);
                if (Weather::ValidUnits(units)) {
                    units = unit;
                    continue;
                }
            }
            out.append(word);
        }
        boost::trim(out);
        q = out;
        return units;
    }

    void listenCmds() {
        SubscriptionRequest sr;
        sr.set_ext(EXTNAME.data());

        CmdEventResponse cer;
        ClientContext context;
        std::unique_ptr<ClientReader<CmdEventResponse>> reader(stub->Commands(&context, sr));
        while (reader->Read(&cer)) {
            IRCEvent e = cer.event().ircevent();
            auto args = cer.event().args();
            //auto chan = cer.event().channel().name();
            auto chan = e.args()[0];
            auto name = cer.name();
            auto q = args["location"];
            std::cout << "Cmd: " << cer.name() << " Chan: " << chan << " Args: " << q << std::endl;
            if (name == "wz" || name == "weather") {
                std::string units = GetUnits(q);
                std::cout << "Units: " << units << " Location: " << q << std::endl;
                if(q == "") {
                    putchan(chan, "Give a location");
                    continue;
                }
                auto loc = std::make_shared<Location>(BING_KEY);
                loc->Lookup(q);
                if (loc->error != "") {
                    putchan(chan, loc->error);
                    continue;
                }
                auto w = Weather(DARKSKY_KEY);
                w.Lookup(loc, units);
                if(w.error != "") {
                    putchan(chan, w.error);
                } else {
                    putchan(chan, w.GetIRC());
                }
            }
            if (name == "snek") {
                putchan(chan, "HSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS");
            }
        }

        if (auto status = reader->Finish(); !status.ok()) {
            std::cout << "reader->Finish rpc failed." << std::endl;
            std::cout << status.error_code() << std::endl;
            std::cout << status.error_message() << std::endl;
        }
    }
private:
    std::vector<std::unique_ptr<Cmd>> cmds_;
};

int main() {
    grpc::SslCredentialsOptions credOpts;

    try {
        DARKSKY_KEY = FileReader("darksky_key").data();
        BING_KEY = FileReader("bing_key").data();
        credOpts.pem_root_certs = FileReader("extra.crt").data();
        credOpts.pem_private_key = FileReader("knivey.key").data();
        credOpts.pem_cert_chain = FileReader("knivey.crt").data();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << '\n';
        exit(-1);
    }

    boost::trim_right(DARKSKY_KEY);
    boost::trim_right(BING_KEY);
    boost::trim_right(credOpts.pem_root_certs);
    boost::trim_right(credOpts.pem_private_key);
    boost::trim_right(credOpts.pem_cert_chain);

    // Create a default SSL ChannelCredentials object.
    auto channel_creds = grpc::SslCredentials(credOpts);

    ChannelArguments cargs;
    cargs.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 130000);
    cargs.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    cargs.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    cargs.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 0);
    cargs.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
    // Create a channel using the credentials created in the previous step.
    while (1) {
        auto channel = grpc::CreateCustomChannel("ext.bitforge.ca:5001", channel_creds, cargs);
        auto client = std::make_unique<ExtClient>(channel);
        // client->putchan("#bots", "testing");
        client->registerCmds();
        client->listenCmds();
        std::this_thread::sleep_for(140s);
    }
    return 0;
}
