#include <dpp/dpp.h>
#include <vector>
#include <map>

#include "snipe.h"

namespace snipe {

    snipe::message snipeCheck(dpp::snowflake channel, snipe::mainCache &cache) {
        snipe::message placeholder;
        placeholder.id = (uint64_t) 0;
        if(cache.lastDeletedMessageId.count(channel)) {
            return cache.messageCache.at(cache.lastDeletedMessageId.at(channel));
        }
        else {
            return placeholder;
        }
    }

    void messageAdd(dpp::message message, snipe::mainCache &cache) {
        snipe::message input;

        input.id = size_t(message.id);
        input.author = message.author;
        input.authorName = message.author.username;

        input.attachments = "";
        if (message.attachments.size() > 0) {
            for (int i = 0; i < message.attachments.size(); i++) {
                input.attachments += message.attachments.at(i).url + "\n";
            }
        }
        else { 
            input.attachments += "[none]";
        }

        try {
            input.contents = message.content;
        }
        catch (const std::exception& e) {
            input.contents = "[empty]";
        }

        cache.messageCache.emplace(size_t(message.id), input);
    }

    void snipeCatch(dpp::snowflake channelId, dpp::snowflake messageId, snipe::mainCache &cache) {
        if(cache.lastDeletedMessageId.count(channelId)) {
            cache.lastDeletedMessageId.at(channelId) = size_t(messageId);
        }
        else {
            cache.lastDeletedMessageId.emplace(channelId, size_t(messageId));
        }
    }

}