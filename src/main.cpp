#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <dpp/dpp.h>
#include <ogg/ogg.h>
#include <opus/opusfile.h>

#include "wordle.h"
#include "snipe.h"

uint64_t botManager = 0;
uint64_t botId = 0;
uint64_t botDebugChannel = 0;

/* message sniping data */
snipe::mainCache cache;
dpp::snowflake snipeCutOff = 0;

/* audio file names */
std::vector<std::string> audioFileNames;

/* wordle game data */
wordle::wordledatabase wordles;
wordle::quordledatabase quordles;

/* preset messages */
std::string helpMessage = "";

/* simple substring search */
bool ssSearch(std::string target, std::string subject) {
    if (target.length() > subject.length()) {
        return false;
    }
    else {
        // simplify query
        for (int i = 0; i < subject.length(); i++) {
            subject[i] = std::tolower(subject[i]);
            if (subject[i] == ' ') {
                subject[i] = '_';
            }
        }
        for (int i = 0; i < target.length(); i++) {
            target[i] = std::tolower(target[i]);
            if (target[i] == ' ') {
                target[i] = '_';
            }
        }

        if (subject.find(target) == subject.npos) {
            return false;
        }
        else {
            return true;
        }
    }
}

int main() {

    /* create cluster */

    std::fstream authFile("dependencies/auth");
    std::string token;
    std::string botManagerInput;
    std::string botIdInput;
    std::string botDebugChannelInput;
    /*
    The file should be:
    bot token
    bot admin id
    bot user id
    debug channel id
    */
    if(authFile.is_open()) {
        std::getline(authFile, token);
        std::getline(authFile, botManagerInput);
        std::getline(authFile, botIdInput);
        std::getline(authFile, botDebugChannelInput);
    }
    else {
        std::cout << "Can't open auth file" << std::endl;
    }

    botManager = std::stoull(botManagerInput);
    botId = std::stoull(botIdInput);
    botDebugChannel = std::stoull(botDebugChannelInput);

    dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content);
    
    authFile.close();

    /* parse long preset messages */
    std::fstream helpFile("dependencies/messages/help");
    if(helpFile.is_open()) {
        std::string helpLine = "";
        while(!helpFile.eof()) {
            std::getline(helpFile, helpLine);
            helpMessage += helpLine + "\n";
        }
    }
    else {
        std::cout << "Can't open auth file" << std::endl;
    }


    /* simple logger */
    bot.on_log(dpp::utility::cout_logger());

    /* slash commands */
    bot.on_slashcommand([](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "snipe") {
            snipe::message sniped = snipe::snipeCheck(event.command.channel_id, cache);

            if (sniped.id != 0) {
                dpp::embed embed = dpp::embed().
                                set_color(dpp::colors::cyan).
                                set_title("Message").
                                set_author(sniped.authorName,
                                    "https://www.example.com/",
                                    (sniped.author).get_avatar_url()).
                                add_field(
                                    "Content",
                                    sniped.contents
                                ).
                                add_field(
                                    "Attachments",
                                    sniped.attachments,
                                    false
                                ).
                                set_footer(dpp::embed_footer().set_text(std::to_string(((dpp::snowflake) sniped.id))));
                                
                event.reply(dpp::message().add_embed(embed));
            }
            else {
                event.reply("Nothing to snipe here");
            }
        }
        if (event.command.get_command_name() == "help") {
            event.reply("```" + helpMessage + "```");
        }
    });

    /* on bot start */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        std::cout << "Logged in as " << bot.me.username << "\n";
        bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_listening, "nothing"));
        
        std::string fileNameInput;
        for (const auto& entry : std::filesystem::directory_iterator("dependencies/audio")) {
            fileNameInput = entry.path().string();
            fileNameInput.erase(0, 19);
            audioFileNames.push_back(fileNameInput);
        }


        bot.message_create(dpp::message(botDebugChannel, ("Bot started.")));

        /* register slash commands */
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("snipe", "snipe message in channel", bot.me.id));
            bot.global_command_create(dpp::slashcommand("help", "bot help", bot.me.id));
        }
    });

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        try {
            std::cout << "[MESSAGE] in " << size_t(event.msg.channel_id) << " from " << event.msg.author.username << " (" << event.msg.author.id << "): " << event.msg.content << std::endl;
            
            //command prefix
            char prefix = '=';
            if ((event.msg.content).at(0) == prefix && event.msg.content != std::string(1, prefix)) {
            
                /* process the message into commands*/
                std::string messagec = event.msg.content;
                messagec.erase(0, 1);
                dpp::snowflake sender = event.msg.author.id;
                std::vector<std::string> commandParts;
                std::stringstream check1(messagec);
                std::string commandPart;
                while (std::getline(check1, commandPart, ' ')) {
                    commandParts.push_back(commandPart);
                }
                messagec.erase();

                /* process commands */

                /* help command */
                if (commandParts.at(0) == "help") {
                    bot.message_create(dpp::message(event.msg.channel_id, ("```" + helpMessage + "```")));
                }

                /* wordle game */
                if (commandParts.at(0) == "wordle") {

                    if ( wordles.data.count(sender) ) {

                        if (commandParts.size() > 1) {

                            if(commandParts.at(1) == "cancel") {
                                bot.message_create(dpp::message(event.msg.channel_id, ("Wordle canceled, answer was " + wordles.data.at(sender).answer)));
                                wordles.data.erase(sender);
                            }

                            else {
                                bot.message_create(dpp::message(event.msg.channel_id, (wordle::wordleCheck(&wordles, sender, commandParts.at(1)))));
                            }

                        }

                        else {
                            bot.message_create(dpp::message(event.msg.channel_id, ("Wordle already set, use 'wordle [word]' to submit answers or 'wordle cancel' to cancel")));
                        }

                    }
                    else if (commandParts.size() > 1) {

                        if (commandParts.at(1) == "harder") {
                            wordle::wordleReg(&wordles, sender, true);
                            bot.message_create(dpp::message(event.msg.channel_id, ("Wordle set with extended answer pool, use 'wordle [word]' to submit answers")));
                        }

                        else {
                            wordle::wordleReg(&wordles, sender, false);
                            bot.message_create(dpp::message(event.msg.channel_id, ("Wordle set, use 'wordle [word]' to submit answers")));
                        }

                    }

                    else {
                        wordle::wordleReg(&wordles, sender, false);
                        bot.message_create(dpp::message(event.msg.channel_id, ("Wordle set, use 'wordle [word]' to submit answers")));
                    }
                }

                /* quordle game */
                if (commandParts.at(0) == "quordle") {

                    if ( quordles.data.count(sender) ) {

                        if (commandParts.size() > 1) {

                            if(commandParts.at(1) == "cancel") {
                                bot.message_create(dpp::message(event.msg.channel_id, ("Quordle canceled, answers were " + 
                                quordles.data.at(sender).answers.at(0) + ", " +
                                quordles.data.at(sender).answers.at(1) + ", " +
                                quordles.data.at(sender).answers.at(2) + ", and " +
                                quordles.data.at(sender).answers.at(3)
                                )));
                                quordles.data.erase(sender);
                            }

                            else {
                                bot.message_create(dpp::message(event.msg.channel_id, (wordle::quordleCheck(&quordles, sender, commandParts.at(1)))));
                            }

                        }

                        else {
                            bot.message_create(dpp::message(event.msg.channel_id, ("Quordle already set, use 'quordle [word]' to submit answers or 'quordle cancel' to cancel")));
                        }

                    }
                    else if (commandParts.size() > 1) {

                        if (commandParts.at(1) == "harder") {
                            wordle::quordleReg(&quordles, sender, true);
                            bot.message_create(dpp::message(event.msg.channel_id, ("Quordle set with extended answer pool, use 'quordle [word]' to submit answers")));
                        }

                        else {
                            wordle::quordleReg(&quordles, sender, false);
                            bot.message_create(dpp::message(event.msg.channel_id, ("Quordle set, use 'quordle [word]' to submit answers")));
                        }

                    }

                    else {
                        wordle::quordleReg(&quordles, sender, false);
                        bot.message_create(dpp::message(event.msg.channel_id, ("Quordle set, use 'quordle [word]' to submit answers")));
                    }

                }
                
                /* snipe deleted message */
                if (commandParts.at(0) == "snipe") {

                    snipe::message sniped = snipe::snipeCheck(event.msg.channel_id, cache);

                    if (sniped.id != 0) {

                        dpp::embed embed = dpp::embed().
                            set_color(dpp::colors::cyan).
                            set_title("Message").
                            set_author(sniped.authorName,
                                "https://www.example.com/",
                                (sniped.author).get_avatar_url()).
                            add_field(
                                "Content",
                                sniped.contents
                            ).
                            add_field(
                                "Attachments",
                                sniped.attachments,
                                false
                            ).
                            set_footer(dpp::embed_footer().set_text(std::to_string(((dpp::snowflake) sniped.id))));

                        bot.message_create(dpp::message(event.msg.channel_id, embed));
                    }
                    else {
                        bot.message_create(dpp::message(event.msg.channel_id, ("Nothing to snipe here")));
                    }

                }



                /* join a voice channel */
                /* adapted from https://dpp.dev/joinvc.html */
                if (commandParts.at(0) == "join") {
                    dpp::guild* g = dpp::find_guild(event.msg.guild_id);
                    auto current_vc = event.from->get_voice(event.msg.guild_id);
                    
                    if (current_vc) {

                        auto users_vc = g->voice_members.find(event.msg.author.id);

                        if (!(users_vc != g->voice_members.end() && current_vc->channel_id == users_vc->second.channel_id)) {
                            event.from->disconnect_voice(event.msg.guild_id);

                            if (!g->connect_member_voice(event.msg.author.id)) {
                                bot.message_create(dpp::message(event.msg.channel_id, "You don't seem to be in a voice channel"));
                            }
                        }
                    }
                    else {

                        if (!g->connect_member_voice(event.msg.author.id)) {
                            bot.message_create(dpp::message(event.msg.channel_id, "You don't seem to be in a voice channel"));
                        }

                    }
                }

                /* leave voice channel */
                if (commandParts.at(0) == "dc") {
                    auto current_vc = event.from->get_voice(event.msg.guild_id);

                    if(current_vc) {
                        event.from->disconnect_voice(event.msg.guild_id);
                    }
                }

                /* stop audio */
                if (commandParts.at(0) == "stop") {
                    dpp::voiceconn* v = event.from->get_voice(event.msg.guild_id);

                    v->voiceclient->stop_audio();
                }

                /* play audio*/
                if (commandParts.at(0) == "play") {
                    //find the file
                    std::string thefilename = "";

                    
                    if (commandParts.size() == 1) {
                        std::srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
                        thefilename = "dependencies/audio/" + audioFileNames.at(std::rand() % audioFileNames.size());
                    }
                    else {
                        std::string filnamecand = event.msg.content;
                        filnamecand.erase(0, 6);
                        for (int i = 0; i < audioFileNames.size(); i++) {
                            if (ssSearch(filnamecand, audioFileNames.at(i))) {
                                thefilename = "dependencies/audio/" + audioFileNames.at(i);
                                break;
                            }
                        }
                    }
                    if(thefilename.length() > 0) {
                        /* Below is from https://dpp.dev/oggopus.html*/

                        dpp::voiceconn* v = event.from->get_voice(event.msg.guild_id);
                        v->voiceclient->stop_audio();

                        ogg_sync_state oy; 
                        ogg_stream_state os;
                        ogg_page og;
                        ogg_packet op;
                        OpusHead header;
                        char *buffer;
            
                        FILE *fd;



                        fd = fopen(thefilename.c_str(), "rb");

                        std::cout << "AUDIO: Playing " << thefilename << std::endl;

                        fseek(fd, 0L, SEEK_END);
                        size_t sz = ftell(fd);
                        rewind(fd);
            
                        ogg_sync_init(&oy);
            
                        int eos = 0;
                        int i;
            
                        buffer = ogg_sync_buffer(&oy, sz);
                        fread(buffer, 1, sz, fd);
            
                        ogg_sync_wrote(&oy, sz);
            
                        if (ogg_sync_pageout(&oy, &og) != 1) {
                            fprintf(stderr,"Does not appear to be ogg stream.\n");
                            exit(1);
                        }
            
                        ogg_stream_init(&os, ogg_page_serialno(&og));
            
                        if (ogg_stream_pagein(&os,&og) < 0) {
                            fprintf(stderr,"Error reading initial page of ogg stream.\n");
                            exit(1);
                        }
            
                        if (ogg_stream_packetout(&os,&op) != 1) {
                            fprintf(stderr,"Error reading header packet of ogg stream.\n");
                            exit(1);
                        }
            
                        /* We must ensure that the ogg stream actually contains opus data */
                        if (!(op.bytes > 8 && !memcmp("OpusHead", op.packet, 8))) {
                            fprintf(stderr,"Not an ogg opus stream.\n");
                            exit(1);
                        }
            
                        /* Parse the header to get stream info */
                        int err = opus_head_parse(&header, op.packet, op.bytes);
                        if (err) {
                            fprintf(stderr,"Not a ogg opus stream\n");
                            exit(1);
                        }
            
                        /* Now we ensure the encoding is correct for Discord */
                        if (header.channel_count != 2 && header.input_sample_rate != 48000) {
                            fprintf(stderr,"Wrong encoding for Discord, must be 48000Hz sample rate with 2 channels.\n");
                            exit(1);
                        }
            
                        /* Now loop though all the pages and send the packets to the vc */
                        while (ogg_sync_pageout(&oy, &og) == 1) {
                            ogg_stream_init(&os, ogg_page_serialno(&og));
            
                            if(ogg_stream_pagein(&os,&og)<0) {
                                fprintf(stderr,"Error reading page of Ogg bitstream data.\n");
                                exit(1);
                            }
            
                            while (ogg_stream_packetout(&os,&op) != 0) {
            
                                /* Read remaining headers */
                                if (op.bytes > 8 && !memcmp("OpusHead", op.packet, 8)) {
                                    int err = opus_head_parse(&header, op.packet, op.bytes);
                                    if (err) {
                                        fprintf(stderr,"Not a ogg opus stream\n");
                                        exit(1);
                                    }
            
                                    if (header.channel_count != 2 && header.input_sample_rate != 48000) {
                                        fprintf(stderr,"Wrong encoding for Discord, must be 48000Hz sample rate with 2 channels.\n");
                                        exit(1);
                                    }
            
                                    continue;
                                }
            
                                /* Skip the opus tags */
                                if (op.bytes > 8 && !memcmp("OpusTags", op.packet, 8))
                                    continue; 
            
                                /* Send the audio */
                                int samples = opus_packet_get_samples_per_frame(op.packet, 48000);
            
                                v->voiceclient->send_audio_opus(op.packet, op.bytes, samples / 48);
                            }
                        }
                        
                        //unload
                        ogg_stream_clear(&os);
                        ogg_sync_clear(&oy);

                        std::cout << "AUDIO: Unloaded " << thefilename << std::endl;
                    }
                }

                /* stop audio */
                if (commandParts.at(0) == "stop") {
                    dpp::voiceconn* v = event.from->get_voice(event.msg.guild_id);

                    v->voiceclient->stop_audio();
                }

                /* debug/admin commands */
                if (size_t(event.msg.author.id) == botManager && size_t(event.msg.channel_id) == botDebugChannel) {
                    /* load in the audio files */
                    if (event.msg.content == "fileloadin") {
                        std::string fileNameInput;
                        for (const auto& entry : std::filesystem::directory_iterator("dependencies/audio")) {
                            fileNameInput = entry.path().string();
                            fileNameInput.erase(0, 19);
                            audioFileNames.push_back(fileNameInput);
                        }
                    }

                    if (commandParts.at(0) == "say" && commandParts.size() > 2) {
                        uint64_t channelIdIn = std::stoull(commandParts.at(1));
                        dpp::snowflake channelId = channelIdIn;
                        std::string msg = event.msg.content;
                        msg.erase(0, 6 + (commandParts.at(1).length()));
                        bot.message_create(dpp::message(channelId, msg));
                    }

                    if (event.msg.content == "set snipe cutoff") {
                        snipeCutOff = event.msg.id;
                        bot.message_create(dpp::message(botDebugChannel, ("Snipe cutoff set")));
                    }
                    
                    if (event.msg.content == "kill bot") {
                        bot.message_create(dpp::message(event.msg.channel_id, ("Killing bot")));
                        dpp::guild* g = dpp::find_guild(-1);
                        
                        std::vector out = g->channels;
                        bot.message_create(dpp::message(event.msg.channel_id, ("Kill failed")));
                    }

                    if (commandParts.at(0) == "vcmove") {

                        uint64_t channelIdIn = std::stoull(commandParts.at(1));
                        dpp::snowflake channelId = channelIdIn;

                        uint64_t guildIdIn = std::stoull(commandParts.at(2));
                        dpp::snowflake guildId = guildIdIn;

                        uint64_t userIdIn = std::stoull(commandParts.at(3));
                        dpp::snowflake userId = userIdIn;

                        bot.guild_member_move(channelId, guildId, userId);
                        bot.message_create(dpp::message(botDebugChannel, ("moved " + commandParts.at(3) + " to " + commandParts.at(1))));
                    }

                }
            }

        }
        catch (const std::exception& e) {
            std::cout << "[ERROR] " << e.what() << std::endl;
        }
        });

    /* deleted message log */
    bot.on_message_delete([&bot](const dpp::message_delete_t& event) {
        if (size_t(event.deleted->author.id) != botId) {

            dpp::snowflake channel = event.deleted->channel_id;
            dpp::snowflake id = event.deleted->id;

            if (size_t(id) > size_t(snipeCutOff)) {
                snipe::snipeCatch(channel, id, cache);
            }

        }
        });

    bot.start(dpp::st_wait);
    
    return 0;
}
