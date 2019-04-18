//
// Created by aidan on 11/7/15.
//

#pragma once

#include <vector>
#include <string>
#include <queue>

#include "botStruct.hh"

namespace discordpp{
	using json = nlohmann::json;
	using snowflake = uint64_t;
	using timePoint = std::chrono::steady_clock::time_point;

	class Bot: virtual BotStruct{
		std::unique_ptr<boost::asio::steady_timer> pacemaker_;
		std::unique_ptr<std::chrono::milliseconds> heartrate_;
		int sequence_ = -1;
		bool gotACK = true;
	public:
		std::multimap<std::string, std::function<void(json)>> handlers;

		Bot(){
			needInit["Bot"] = true;
		}

		void
		initBot(unsigned int apiVersionIn, const std::string& tokenIn, std::shared_ptr<boost::asio::io_context> aiocIn){
			apiVersion = apiVersionIn;
			token = tokenIn;
			aioc = aiocIn;
			needInit["Bot"] = false;
		}

	protected:
		void sendHeartbeat(){
			if(!gotACK){
				std::cerr << "Discord Servers did not respond to heartbeat. Reconnect not implemented.\n";
				exit(1);
			}
			gotACK = false;
			std::cout << "Sending heartbeat..." << std::endl;
			pacemaker_ = std::make_unique<boost::asio::steady_timer>(
					*aioc,
					std::chrono::steady_clock::now() + *heartrate_
			);
			pacemaker_->async_wait(
					[this](const boost::system::error_code){
						sendHeartbeat();
					}
			);
			if(sequence_ >= 0){
				send(1, sequence_);
			}else{
				send(1, {});
			}
		}

		void receivePayload(json payload) override{
			//std::cerr << "Recieved Payload: " << payload.dump(4) << '\n';

			switch(payload["op"].get<int>()){
				case 0:  // Dispatch:           dispatches an event
					sequence_ = payload["s"].get<int>();
					if(handlers.find(payload["t"]) == handlers.end()){
						std::cerr << "No handlers defined for " << payload["t"] << "\n";
					}else{
						for(auto handler = handlers.lower_bound(payload["t"]);
						    handler != handlers.upper_bound(payload["t"]); handler++){
							handler->second(payload["d"]);
						}
					}
					break;
				case 1:  // Heartbeat:          used for ping checking
					std::cerr << "Discord Servers requested a heartbeat, which is not implemented.\n";
					break;
				case 7:  // Reconnect:          used to tell clients to reconnect to the gateway
					std::cerr << "Discord Servers requested a reconnect. Reconnect not implemented.";
					exit(1);
				case 9:  // Invalid Session:	used to notify client they have an invalid session id
					std::cerr << "Discord Servers notified of an invalid session ID. Reconnect not implemented.";
					exit(1);
				case 10: // Hello:              sent immediately after connecting, contains heartbeat and server debug information
					heartrate_ = std::make_unique<std::chrono::milliseconds>(payload["d"]["heartbeat_interval"]);
					sendHeartbeat();
					send(
							2, {
									{"token",      token},
									{
									 "properties", {
											               {"$os", "linux"},
											               {"$browser", "discordpp"},
											               {"$device", "discordpp"},
									               }
									}
							}
					);
					break;
				case 11: // Heartbeat ACK:      sent immediately following a client heartbeat that was received
					gotACK = true;
					std::cout << "Heartbeat Successful." << std::endl;
					break;
				default:
					std::cerr << "Unexpected opcode " << payload["op"] << "! Message:\n"
					          << payload.dump(4) << '\n';
			}
		}

		// Ratelimiting
	protected:
		bool calling = false;
		std::shared_ptr<boost::asio::steady_timer> nextCall;
		std::priority_queue<std::pair<timePoint, std::function<void()>>> callQueue;
		std::map<std::string, timePoint> limited;

	public:
	private:
		void
		call(std::string requestType, std::string targetURL, json body, std::function<void(json)> callback) override{
			const std::string bucket = getBucket(targetURL);

			if(calling){
				callQueue.emplace(
						std::chrono::steady_clock::now(),
						[this, requestType, targetURL, body, callback](){
							call(requestType, targetURL, body, callback);
						}
						);
			}
			if(limited.find(bucket) != limited.end()){
				if(limited[bucket] < std::chrono::steady_clock::now()){
					limited.erase(bucket);
				}else{

				}
			}
			json res = doCall(requestType, targetURL, body);
			nextCall->async_wait(
					[this, retryTimer, requestType, targetURL, body](const boost::system::error_code){
						call(requestType, targetURL, body);
					}
			);
			callback(res);
		}

	public:

		void ratelimited(bool global, unsigned long long retryAfter, std::string requestType, std::string targetURL,
		                 json body) override{
			timePoint retry =
					std::chrono::steady_clock::now() +
					std::chrono::milliseconds(retryAfter);
			std::string bucket = global ? "global" : getBucket(targetURL);
			limited[bucket] = retry;

			std::shared_ptr<boost::asio::steady_timer> retryTimer = std::make_shared<boost::asio::steady_timer>(
					*aioc,
					std::chrono::steady_clock::now() + *heartrate_
			);

			retryTimer->async_wait(
					[this, retryTimer, requestType, targetURL, body](const boost::system::error_code){
						call(requestType, targetURL, body);
					}
			);
		}
	};
}
