#include <string>
#include <numeric>

#include <iostream>

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

            /* make sure e is prime */
            for(long long i = 2; i < e/2; i++) {
                if (e % i == 0) {
                    return ":x: Not a valid key";
                }
            }

            long long p = 0;
            long long q = 0;


            for(long long i = 3; i < n; i+=2) {
                if (n % i == 0) {
                    /* store the multiples */
                    p = i;
                    q = n / p;
                    break;
                }
            }

            /* make sure p and q are prime */
            for(long long i = 2; i < p; i++) {
                if (p % i == 0) {
                    return ":x: Not a valid key";
                }
            }
            for(long long i = 2; i < q; i++) {
                if (q % i == 0) {
                    return ":x: Not a valid key";
                }
            }

            long long lcm = (p - 1) * ((q - 1) / std::gcd<int64_t>(p - 1, q - 1));

            if(n % 2 == 0 || n < 2 || e < 2 || e > lcm || lcm % e == 0) {return ":x: Not a valid key";}

            long long x, y;
            euclid(e, lcm, x, y);
            return std::to_string((y + lcm) % lcm);
        }
        catch (const std::exception& e) {
            return ":x: Not a valid key";
        }
    }
}