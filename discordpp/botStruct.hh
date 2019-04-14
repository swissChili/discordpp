//
// Created by Aidan on 6/21/2018.
//

#pragma once

#include <iostream>
#include <regex>

#include <boost/asio.hpp>

#include <nlohmann/json.hpp>

namespace discordpp{
	using json = nlohmann::json;

	struct ratelimit{
		int millis;
	};

	class BotStruct{
	public:
		virtual ~BotStruct(){};

		virtual json call(std::string requestType, std::string targetURL, json body = {}) = 0;

		virtual void send(int opcode, json payload = {}) = 0;

		void run(){
			bool ready = true;
			for(auto module: needInit){
				if(module.second){
					std::cerr << "Forgot to initialize: " << module.first << '\n';
					ready = false;
				}
			}
			if(ready){
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

		std::map<std::string, bool> needInit;
		unsigned int apiVersion = 6;
		std::shared_ptr<boost::asio::io_context> aioc;
		std::string token;
	};

	std::string getBucket(std::string toBucket){
		static std::vector<std::pair<std::regex, std::string>> buckets;
		if(buckets.empty()){
			std::string src[]{
					"GET/guilds/{guild.id}/audit-logs",
					"GET/channels/{channel.id}",
					"PUT/channels/{channel.id}",
					"PATCH/channels/{channel.id}",
					"DELETE/channels/{channel.id}",
					"GET/channels/{channel.id}/messages",
					"GET/channels/{channel.id}/messages/{message.id}",
					"POST/channels/{channel.id}/messages",
					"PUT/channels/{channel.id}/messages/{message.id}/reactions/{emoji}/@me",
					"DELETE/channels/{channel.id}/messages/{message.id}/reactions/{emoji}/@me",
					"DELETE/channels/{channel.id}/messages/{message.id}/reactions/{emoji}/{user.id}",
					"GET/channels/{channel.id}/messages/{message.id}/reactions/{emoji}",
					"DELETE/channels/{channel.id}/messages/{message.id}/reactions",
					"PATCH/channels/{channel.id}/messages/{message.id}",
					"DELETE/channels/{channel.id}/messages/{message.id}",
					"POST/channels/{channel.id}/messages/bulk-delete",
					"POST/channels/{channel.id}/messages/bulk_delete",
					"PUT/channels/{channel.id}/permissions/{overwrite.id}",
					"GET/channels/{channel.id}/invites",
					"POST/channels/{channel.id}/invites",
					"DELETE/channels/{channel.id}/permissions/{overwrite.id}",
					"POST/channels/{channel.id}/typing",
					"GET/channels/{channel.id}/pins",
					"PUT/channels/{channel.id}/pins/{message.id}",
					"DELETE/channels/{channel.id}/pins/{message.id}",
					"PUT/channels/{channel.id}/recipients/{user.id}",
					"DELETE/channels/{channel.id}/recipients/{user.id}",
					"GET/guilds/{guild.id}/emojis",
					"GET/guilds/{guild.id}/emojis/{emoji.id}",
					"POST/guilds/{guild.id}/emojis",
					"PATCH/guilds/{guild.id}/emojis/{emoji.id}",
					"DELETE/guilds/{guild.id}/emojis/{emoji.id}",
					"POST/guilds",
					"GET/guilds/{guild.id}",
					"PATCH/guilds/{guild.id}",
					"DELETE/guilds/{guild.id}",
					"GET/guilds/{guild.id}/channels",
					"POST/guilds/{guild.id}/channels",
					"PATCH/guilds/{guild.id}/channels",
					"GET/guilds/{guild.id}/members/{user.id}",
					"GET/guilds/{guild.id}/members",
					"PUT/guilds/{guild.id}/members/{user.id}",
					"PATCH/guilds/{guild.id}/members/{user.id}",
					"PATCH/guilds/{guild.id}/members/@me/nick",
					"PUT/guilds/{guild.id}/members/{user.id}/roles/{role.id}",
					"DELETE/guilds/{guild.id}/members/{user.id}/roles/{role.id}",
					"DELETE/guilds/{guild.id}/members/{user.id}",
					"GET/guilds/{guild.id}/bans",
					"GET/guilds/{guild.id}/bans/{user.id}",
					"PUT/guilds/{guild.id}/bans/{user.id}",
					"DELETE/guilds/{guild.id}/bans/{user.id}",
					"GET/guilds/{guild.id}/roles",
					"POST/guilds/{guild.id}/roles",
					"PATCH/guilds/{guild.id}/roles",
					"PATCH/guilds/{guild.id}/roles/{role.id}",
					"DELETE/guilds/{guild.id}/roles/{role.id}",
					"GET/guilds/{guild.id}/prune",
					"POST/guilds/{guild.id}/prune",
					"GET/guilds/{guild.id}/regions",
					"GET/guilds/{guild.id}/invites",
					"GET/guilds/{guild.id}/integrations",
					"POST/guilds/{guild.id}/integrations",
					"PATCH/guilds/{guild.id}/integrations/{integration.id}",
					"DELETE/guilds/{guild.id}/integrations/{integration.id}",
					"POST/guilds/{guild.id}/integrations/{integration.id}/sync",
					"GET/guilds/{guild.id}/embed",
					"PATCH/guilds/{guild.id}/embed",
					"GET/guilds/{guild.id}/vanity-url",
					"GET/guilds/{guild.id}/widget.png",
					"GET/invites/{invite.code}",
					"DELETE/invites/{invite.code}",
					"GET/users/@me",
					"GET/users/{user.id}",
					"PATCH/users/@me",
					"GET/users/@me/guilds",
					"DELETE/users/@me/guilds/{guild.id}",
					"GET/users/@me/channels",
					"POST/users/@me/channels",
					"POST/users/@me/channels",
					"GET/users/@me/connections",
					"GET/voice/regions",
					"POST/channels/{channel.id}/webhooks",
					"GET/channels/{channel.id}/webhooks",
					"GET/guilds/{guild.id}/webhooks",
					"GET/webhooks/{webhook.id}",
					"GET/webhooks/{webhook.id}/{webhook.token}",
					"PATCH/webhooks/{webhook.id}",
					"PATCH/webhooks/{webhook.id}/{webhook.token}",
					"DELETE/webhooks/{webhook.id}",
					"DELETE/webhooks/{webhook.id}/{webhook.token}",
					"POST/webhooks/{webhook.id}/{webhook.token}",
					"POST/webhooks/{webhook.id}/{webhook.token}/slack",
					"POST/webhooks/{webhook.id}/{webhook.token}/github",
			};
			std::regex majorParam(R"(\{(channel\.id|guild\.id|webhook\.id)\})");
			std::regex anyParam(R"(\{.+?\})");
			for(const std::string& path : src){
				std::string pattern = std::regex_replace(
						std::regex_replace(path, majorParam, "(.+?)"),
						anyParam,
						".+?"
				);
				std::string replace = path;
				for(int i = 1; std::regex_match(path, majorParam); i++){
					std::regex_replace(path, majorParam, "$" + std::to_string(i));
				}
				buckets.emplace_back(pattern, replace);
			}
		}
		static std::regex doubleSlash = std::regex("//");
		toBucket = std::regex_replace(toBucket, doubleSlash, "/");
		for(const auto& bucket : buckets){
			if(std::regex_match(toBucket, bucket.first)){
				return std::regex_replace(toBucket, bucket.first, bucket.second);
			}
		}
		throw "Invalid path";
	}
}

