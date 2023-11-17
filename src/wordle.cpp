#include <dpp/dpp.h>
#include <vector>

#include "wordle.h"

namespace wordle {

    void wordleReg(wordle::wordledatabase* database, dpp::snowflake player, bool hardmode) {
        std::string ans = "aaaaa";
        std::srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        int index;
        if (hardmode) {
            std::fstream wordlist("dependencies/wordle-allowed-guesses");
            index = (std::rand() % 12966) + 1;
            for (int i = 0; i < index; i++) {
                std::getline(wordlist, ans);
            }
        }
        else {
            std::fstream anslist("dependencies/wordle-anspool");
            index = (std::rand() % 2310) + 1;
            for (int i = 0; i < index; i++) {
                std::getline(anslist, ans);
            }
        }
        wordle::wordlegame thegame;

        thegame.answer = ans;
        thegame.hardmode = hardmode;
        thegame.msgdata = "";
        thegame.guesses = 0;
        database->data.emplace(player, thegame);
    }

    std::string wordleCheck(wordle::wordledatabase* database, dpp::snowflake sender, std::string sub) {
        bool valguess = false;
        std::string guessreader;
        std::fstream wordlist("dependencies/wordle-allowed-guesses");
        for (int i = 0; i < 12966; i++) {
            std::getline(wordlist, guessreader);
            if (guessreader == sub) {
                valguess = true;
                break;
            }
        }
        wordle::wordlegame thegame = database->data.at(sender);

        if (valguess) {
            std::string ans = thegame.answer;
            thegame.guesses++;
            std::string msg = "<@!" + std::to_string(sender) + "> - try " + std::to_string(thegame.guesses) + "/6";
            if (thegame.hardmode) {
                msg.append(" (extended answer pool)");
            }
            msg.append("\n");
            std::map<char, int> letterpool;
            bool alletter = false;
            for (int i = 0; i < 5; i++) {
                if (letterpool.count(ans.at(i))) {
                    letterpool.at(ans.at(i))++;
                }
                if (!alletter) {
                    letterpool.emplace(ans.at(i), 1);
                }
            }

            int corletters = 0;
            int letmat[] = { 0, 0, 0, 0, 0 };
            for (int i = 0; i < 5; i++) {
                if (sub.at(i) == ans.at(i)) {
                    letmat[i]++;
                }
            }

            for (int i = 0; i < 5; i++) {
                if (letterpool.count(sub.at(i))) {
                    if (sub.at(i) == ans.at(i)) {
                        thegame.msgdata.append(":green_square: ");
                        corletters++;
                    }
                    else {
                        bool verdict = true;
                        bool otherletter = false;
                        int lettersleft = letterpool.at(sub.at(i));
                        for (int q = 0; q < 5; q++) {
                            if (letmat[q] == 1 && sub.at(q) == sub.at(i)) {
                                otherletter = true;
                                lettersleft--;
                            }
                        }
                        if (lettersleft == 0) {
                            verdict = false;
                        }
                        if (verdict) {
                            thegame.msgdata.append(":yellow_square: ");
                            letterpool.at(sub.at(i))--;
                        }
                        else {
                            thegame.msgdata.append(":black_large_square: ");
                        }
                    }
                }
                else {
                    thegame.msgdata.append(":black_large_square: ");
                }
                alletter = false;
            }

            thegame.msgdata.append(" - " + sub + "\n");
            msg.append(thegame.msgdata);
            database->data.at(sender) = thegame;
            if (corletters == 5) {
                msg.append("Wordle complete");
                database->data.erase(sender);
            }
            else if (thegame.guesses > 5) {
                msg.append("Answer was " + thegame.answer);
                database->data.erase(sender);
            }
            return msg;
        }
        else {
            if (sub.length() == 5) {
                return ":x: Guess not in word list";
            }
            else {
                return ":x: Must be 5 letters";
            }
        }
    }

    std::pair<std::string, int> singleLineTest(std::string ans, std::string sub) {
        std::string rets = "";
        int win = 0;
        std::map<char, int> letterpool;
        bool alletter = false;
        for (int i = 0; i < 5; i++) {
            if (letterpool.count(ans.at(i))) {
                letterpool.at(ans.at(i))++;
            }
            else {
                letterpool.emplace(ans.at(i), 1);
            }
        }

        int corletters = 0;
        int letmat[] = { 0, 0, 0, 0, 0 };
        for (int i = 0; i < 5; i++) {
            if (sub.at(i) == ans.at(i)) {
                letmat[i]++;
            }
        }
        for (int i = 0; i < 5; i++) {
            if (letterpool.count(sub.at(i))) {
                if (sub.at(i) == ans.at(i)) {
                    rets.append("G ");
                    corletters++;
                }
                else {
                    bool verdict = true;
                    bool otherletter = false;
                    int lettersleft = letterpool.at(sub.at(i));
                    for (int q = 0; q < 5; q++) {
                        if (letmat[q] == 1 && sub.at(q) == sub.at(i)) {
                            otherletter = true;
                            lettersleft--;
                        }
                    }
                    if (lettersleft == 0) {
                        verdict = false;
                    }
                    if (verdict) {
                        rets.append("Y ");
                        letterpool.at(sub.at(i))--;
                    }
                    else {
                        rets.append("B ");
                    }
                }
            }
            else {
                rets.append("B ");
            }
            alletter = false;
        }
        if (corletters == 5) {
            win = 1;
        }
        std::pair<std::string, int> finr;
        finr.first = rets;
        finr.second = win;
        return finr;
    }

    void quordleReg(wordle::quordledatabase* database, dpp::snowflake sender, bool extenpool) {
        std::vector<std::string> ans;
        int index;
        if (extenpool) {
            for (int q = 0; q < 4; q++) {
                std::fstream wordlist("dependencies/wordle-allowed-guesses");
                std::srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + q);
                index = (std::rand() % 12966) + 1;
                std::string ansin;
                for (int i = 0; i < index; i++) {
                    std::getline(wordlist, ansin);
                }
                ans.push_back(ansin);
            }
        }
        else {
            for (int q = 0; q < 4; q++) {
                std::fstream anslist("dependencies/wordle-anspool");
                std::srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + q);
                index = (std::rand() % 2310) + 1;
                std::string ansin;
                for (int i = 0; i < index; i++) {
                    std::getline(anslist, ansin);
                }
                ans.push_back(ansin);
            }
        }

        wordle::quordlegame thegame;
        thegame.answers = ans;
        thegame.hardmode = extenpool;
        thegame.guesses = 0;
        thegame.msgdata = "";
        thegame.findata = { false, false, false, false };

        database->data.emplace(sender, thegame);
    }

    std::string quordleCheck(wordle::quordledatabase* database, dpp::snowflake sender, std::string sub) {
        bool valguess = false;
        std::string guessreader;
        std::fstream wordlist("dependencies/wordle-allowed-guesses");
        for (int i = 0; i < 12966; i++) {
            std::getline(wordlist, guessreader);
            if (guessreader == sub) {
                valguess = true;
                break;
            }
        }

        wordle::quordlegame thegame = database->data.at(sender);

        if (valguess) {
            std::vector<std::string> ans = thegame.answers;
            thegame.guesses++;
            std::string msg = "<@!" + std::to_string(sender) + "> - try " + std::to_string(thegame.guesses) + "/9";
            if (thegame.hardmode) {
                msg.append(" (extended answer pool)");
            }
            msg.append("\n");

            for (int i = 0; i < 4; i++) {
                if (thegame.findata.at(i)) {
                    thegame.msgdata.append("B B B B B | ");
                }
                else {
                    thegame.msgdata.append(singleLineTest(ans.at(i), sub).first + "| ");
                    thegame.findata.at(i) = singleLineTest(ans.at(i), sub).second;
                }
            }

            thegame.msgdata.append("- " + sub + "\n");

            msg.append(thegame.msgdata);

            database->data.at(sender) = thegame;

            if (thegame.findata.at(0) && thegame.findata.at(1) && thegame.findata.at(2) && thegame.findata.at(3)) {
                msg.append("Quordle complete");
                database->data.erase(sender);
            }
            else if (thegame.guesses > 9) {
                msg.append("Answers were " + thegame.answers.at(0) + ", " + thegame.answers.at(1) + ", " + thegame.answers.at(2) + ", and " + thegame.answers.at(3));
                database->data.erase(sender);
            }
            return msg;
        }
        else {
            if (sub.length() == 5) {
                return ":x: Guess not in word list";
            }
            else {
                return ":x: Must be 5 letters";
            }
        }
    }
}