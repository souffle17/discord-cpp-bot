#pragma once
#include <dpp/dpp.h>

namespace snipe {
    struct message {
        uint64_t id;
        dpp::user author;
        std::string authorName;
        std::string attachments;
        std::string contents;
    };
    struct mainCache {
        std::map<dpp::snowflake, uint64_t> lastDeletedMessageId;
        std::map<uint64_t, snipe::message> messageCache;
    };

    snipe::message snipeCheck(dpp::snowflake channel, snipe::mainCache &cache);

    void messageAdd(dpp::message message, snipe::mainCache &cache);

    void snipeCatch(dpp::snowflake channelId, dpp::snowflake messageId, snipe::mainCache &cache);
    
};