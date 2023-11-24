#pragma once
#include <string>

namespace rsa {
    /* manually crunch private key from a public key */
    std::string reverseKey (std::string pk, std::string pe);

    /* encrypt something */
    std::string encrypt (std::string message, std::string m, std::string e);
    
    /* decrypt something */
    std::string decrypt (std::vector<std::string> message, std::string m, std::string e);
}