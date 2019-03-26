//
// Created by Aidan on 6/21/2018.
//

#ifndef EXAMPLE_BOT_BOTREQUIRED_HH
#define EXAMPLE_BOT_BOTREQUIRED_HH

#include <iostream>
#include <utility>

#include <boost/asio.hpp>

#include <nlohmann/json.hpp>

namespace discordpp {
    using json = nlohmann::json;

    typedef std::function<void(json)> callback;

    struct ratelimit{
        int millis;
    };

    class BotStruct {
    public:
        virtual ~BotStruct() = default;
        virtual json call(std::string requestType, std::string targetURL, json body = {}){
            return doCall(requestType, targetURL, body).first;
        };
        virtual void send(int opcode, json payload = {}){
            return doSend(opcode, payload);
        };

        void run(){
            bool ready = true;
            for(auto module: needInit){
                if(module.second){
                    std::cerr << "Forgot to initialize: " << module.first << '\n';
                    ready = false;
                }
            }
            if(ready) {
                runctd();
            }
        }

    protected:
        virtual void runctd(){
            std::cerr << "Starting run loop" << '\n';
            aioc->run();
            std::cerr << "Ending run loop" << '\n';
        }

        virtual void recievePayload(json payload) = 0;
        virtual std::pair<json, std::string> doCall(std::string requestType, std::string targetURL, json body = {}) = 0;
        virtual void doSend(int opcode, json payload = {}) = 0;

        std::map<std::string, bool> needInit;
        unsigned int apiVersion = 6;
        std::shared_ptr<boost::asio::io_context> aioc;
        std::string token;
    };
}

#endif //EXAMPLE_BOT_BOTREQUIRED_HH
