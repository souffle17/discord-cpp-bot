#pragma once
#include <dpp/dpp.h>
#include <vector>

namespace wordle {
    /* a wordle game */
    struct wordlegame {
        std::string answer;
        bool hardmode;
        std::string msgdata;
        int guesses;
    };

    /* a store of wordle games by user */
    struct wordledatabase {
        std::map<dpp::snowflake, wordle::wordlegame> data;
    };

    /* a quordle game */
    struct quordlegame {
        std::vector<std::string> answers;
        bool hardmode;
        std::string msgdata;
        int guesses;
        std::vector<bool> findata;
    };

    /* a store of quordle games by user */
    struct quordledatabase {
        std::map<dpp::snowflake, wordle::quordlegame> data;
    };

    /* make a new wordle game */
    void wordleReg(wordle::wordledatabase* database, dpp::snowflake player, bool hardmode);

    /* run a check on a word in a wordle game */
    std::string wordleCheck(wordle::wordledatabase* database, dpp::snowflake sender, std::string sub);

    /* run a word check for a single line */
    std::pair<std::string, int> singleLineTest(std::string ans, std::string sub);

    /* make a new wordle game */
    void quordleReg(wordle::quordledatabase* database, dpp::snowflake sender, bool extenpool);

    /* run a check on a word in a quordle game */
    std::string quordleCheck(wordle::quordledatabase* database, dpp::snowflake sender, std::string sub);

};
    