#include <boost/algorithm/string.hpp>
using namespace boost;

#include "location.h"
#include "weather.h"
#include <fmt/core.h>
#include <iostream>
#include <string>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <unistd.h>
#include <vector>

#include "grpc/ultimateq.grpc.pb.h"
#include "grpc/ultimateq.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using namespace api;

using namespace std;

const char *EXTNAME = "weather";
const char *NETNAME = "zkpq";

static string DARKSKY_KEY;

class ExtClient {
  public:
    ExtClient(std::shared_ptr<Channel> channel) : stub(Ext::NewStub(channel)) {
        ;
    }
    std::unique_ptr<Ext::Stub> stub;

    void registerPubCmd(string name, string desc, vector<string> args) {
        Cmd *cmd = new Cmd();
        cmd->set_ext(EXTNAME);
        cmd->set_name(name);
        cmd->set_desc(desc);
        cmd->set_kind(Cmd::AnyKind);
        cmd->set_scope(Cmd::Public);
        for (auto &a : args) {
            auto Args = cmd->add_args();
            *Args = a;
        }

        RegisterCmdRequest cmdr;
        cmdr.set_ext(EXTNAME);
        cmdr.set_allocated_cmd(cmd);

        ClientContext context;
        RegisterResponse RegResp;
        Status status = stub->RegisterCmd(&context, cmdr, &RegResp);
        if (!status.ok()) {
            std::cout << "RegisterCmd rpc failed." << std::endl;
            cout << status.error_code() << endl;
            cout << status.error_message() << endl;
        }
    }

    void registerCmds() {
        registerPubCmd("wz", "check the weather", {"location..."});
        registerPubCmd("weather", "check the weather", {"location..."});
        registerPubCmd("snek", "hssss", {});
    }

    void putchan(string chan, string msg) {
        string rawmsg = fmt::format("PRIVMSG {} :{}", chan, msg);
        cout << "<< " << rawmsg << endl;
        WriteRequest wr;
        wr.set_ext(EXTNAME);
        wr.set_net(NETNAME);
        wr.set_msg(rawmsg);
        ClientContext context;
        Empty resp;
        Status status = stub->Write(&context, wr, &resp);
        if (!status.ok()) {
            std::cout << "Write rpc failed." << std::endl;
            cout << status.error_code() << endl;
            cout << status.error_message() << endl;
            return;
        }
    }
    
    string GetUnits(string &q) {
        string units = "auto";
        string out = "";
        vector<string> words;
        split(words, q, is_space());
        for(string word : words) {
            if (word.substr(0, 2) == "--") {
                string unit = word.substr(2, string::npos);
                if (Weather::ValidUnits(units)) {
                    units = unit;
                    continue;
                }
            }
            out.append(word);
        }
        trim(out);
        q = out;
        return units;
    }

    void listenCmds() {
        SubscriptionRequest sr;
        sr.set_ext(EXTNAME);

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
            std::cout << "Cmd: " << cer.name() << " Chan: " << chan << " Args: " << q << endl;
            if (name == "wz" || name == "weather") {
                string units = GetUnits(q);
                std::cout << "Units: " << units << "Location: " << q << std::endl;
                if(q == "") {
                    putchan(chan, "Give a location");
                    continue;
                }
                auto loc = Location(q);
                if (loc.error != "") {
                    putchan(chan, loc.error);
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
        Status status = reader->Finish();
        if (!status.ok()) {
            std::cout << "reader->Finish rpc failed." << std::endl;
            cout << status.error_code() << endl;
            cout << status.error_message() << endl;
            return;
        }
    }
};

string File2String(string filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    string out = buffer.str();
    trim_right(out);
    return out;
}

int main() {
    DARKSKY_KEY = File2String("darksky_key");
    
    grpc::SslCredentialsOptions credOpts;
    credOpts.pem_root_certs = File2String("extra.crt");
    credOpts.pem_private_key = File2String("knivey.key");
    credOpts.pem_cert_chain = File2String("knivey.crt");

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
        auto client = new ExtClient(channel);
        // client->putchan("#bots", "testing");
        client->registerCmds();
        client->listenCmds();
        sleep(140);
    }
    return 0;
}
