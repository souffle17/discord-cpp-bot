#include <string>
#include <numeric>
#include <chrono>
#include <gmpxx.h>

#include <iostream>

// https://en.wikipedia.org/wiki/RSA_(cryptosystem)

void euclid (mpz_class a, mpz_class b, mpz_class &x, mpz_class &y){
	if(a == 0) {
        x = 1;
        y = 0;
    }
    else {
        euclid (b % a, a, x, y);
        mpz_class oldY = y;
        y = x - (b / a) * y;
        x = oldY;
    }
}

mpz_class bigPow (mpz_class a, mpz_class b) {
    mpz_class out = 1;
    for(mpz_class i = 0; i < b; i++) {
        out *= a;
    }
    return out;
}

namespace rsa {
    std::string reverseKey (std::string pk, std::string pe) {
        try {
            mpz_class n;
            mpz_class e;

            try {
                n = pk;
                e = pe;
            }
            catch (const std::exception& error) {
                std::cout << error.what();
                return ":x: Not a valid key";
            }

            /* make sure n and e arent too small */
            if(n < 2 || e < 1) {return ":x: Not a valid key";}

            /* make sure e is prime */
            for(mpz_class i = 2; i < e/2 + 1; i++) {
                if (e % i == 0) {
                    return ":x: Not a valid key";
                }
            }

            auto startTime = std::chrono::system_clock::now();

            mpz_class p = 2;
            mpz_class q = 0;

            mpz_class max = n / 2;

            mpz_class maxLimit = mpz_class("10000000000000000000");


            if(maxLimit < n / 2) 
                max = maxLimit;
            if(n % 2 == 1) {
                for(mpz_class i = 3; i < max; i+=2) {
                    if (n % i == 0) {
                        /* store the multiple */
                        p = i;
                        break;
                    }
                }
                if(p == 2) {
                    if(max == maxLimit)
                        return ":x: That will take too long";
                        
                    return ":x: Not a valid key";
                }
            }
            q = n / p;
            

            mpz_class totient = lcm((p - 1), (q - 1));

            /* make sure modular multiplicative inverse can be taken */
            if(e < 2 || e > totient || totient % e == 0) {return ":x: Not a valid key";}

            mpz_class x, y;
            euclid(e, totient, x, y);

            mpz_class privateKey = (y + totient) % totient;

            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - startTime;

            return privateKey.get_str() + "\nTook " + std::to_string(elapsed_seconds.count()) + " seconds";
        }
        catch (const std::exception& e) {
            std::cout << e.what();
            return ":x: Not a valid key";
        }
    }

    std::string encrypt (std::string message, std::string m, std::string e) {
        try {
            mpz_class modulus = mpz_class(m);
            mpz_class exponent = mpz_class(e);

            if(m.length() > 19 || e.length() > 19) {
                return ":x: That will take too long";
            }

            std::vector<mpz_class> processing;

            for(int i = 0; i < message.length(); i++) {
                mpz_class character = (int) message.at(i);

                mpz_class output = bigPow(character, exponent) % modulus;
                processing.push_back(output);
            }

            std::string out = "";

            for(int i = 0; i < processing.size(); i++) {
                out += processing.at(i).get_str() + " ";
            }
            
            return out.substr(0, out.length()-1);
        }
        catch (const std::exception& e) {
            std::cout << e.what();
            return ":x: Bad input";
        }
    }

    std::string decrypt (std::vector<std::string> message, std::string m, std::string e) {
        try {
            mpz_class modulus = mpz_class(m);
            mpz_class exponent = mpz_class(e);

            if(m.length() > 19 || e.length() > 19) {
                return ":x: That will take too long";
            }

            std::string out = "";

            std::vector<mpz_class> processing;

            mpz_class encrypted;
            mpz_class decrypted;

            for(int i = 4; i < message.size(); i++) {
                encrypted = mpz_class(message.at(i));

                decrypted = bigPow(encrypted, exponent) % modulus;

                out += char(std::stoi(decrypted.get_str()));
            }
            return "Message: " + out;
        }
        catch (const std::exception& e) {
            std::cout << e.what();
            return ":x: Bad input";
        }
    }
}