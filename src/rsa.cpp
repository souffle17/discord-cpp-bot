#include <string>
#include <numeric>
#include <chrono>

void euclid (long long a, long long b, long long &x, long long &y){
	if(a == 0) {
        x = 1;
        y = 0;
    }
    else {
        euclid (b % a, a, x, y);
        long long oldY = y;
        y = x - (b / a) * y;
        x = oldY;
    }
}

namespace rsa {
    std::string reverseKey (std::string pk, std::string pe) {
        try {
            /* make sure it's a number */
            for (int i = 0; i < pk.length(); i++) {
                if(isdigit(pk.at(i)) == 0) {
                    return ":x: Not a valid key";
                }
            }
            for (int i = 0; i < pe.length(); i++) {
                if(isdigit(pe.at(i)) == 0) {
                    return ":x: Not a valid key";
                }
            }

            /* make sure it's not too long */
            if(pk.length() > 18) {
                return ":x: That will take too long";
            }

            long long n = std::stoll(pk);
            long long e = std::stoll(pe);

            /* make sure n isnt too small */
            if(n < 2) {return ":x: Not a valid key";}

            /* make sure e is prime */
            for(long long i = 2; i < e/2; i++) {
                if (e % i == 0) {
                    return ":x: Not a valid key";
                }
            }

            auto startTime = std::chrono::system_clock::now();

            long long p = 2;
            long long q;

            long long max = n / p;

            if(n % 2 == 1) {
                for(long long i = 3; i < max; i+=2) {
                    if (n % i == 0) {
                        if(p == 2) {
                            /* store the multiple */
                            p = i;
                            max = n / p;
                        }
                        else {
                            return ":x: Not a valid key";
                        }
                    }
                }
            }
            q = n / p;
            

            long long lcm = (p - 1) * ((q - 1) / std::gcd<int64_t>(p - 1, q - 1));

            /* make sure modular multiplicative inverse can be taken */
            if(e < 2 || e > lcm || lcm % e == 0) {return ":x: Not a valid key";}

            long long x, y;
            euclid(e, lcm, x, y);

            long long privateKey = (y + lcm) % lcm;

            auto endTime = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = endTime - startTime;

            return std::to_string(privateKey) + "\nTook " + std::to_string(elapsed_seconds.count()) + " seconds";
        }
        catch (const std::exception& e) {
            return ":x: Not a valid key";
        }
    }
}