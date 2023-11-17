#pragma once
#include <string>

namespace rsa {
    /* manually crunch private key from a public key */
    std::string reverseKey (std::string pk, std::string pe);
}