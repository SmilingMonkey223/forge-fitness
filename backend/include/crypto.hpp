#pragma once

#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace forge {

class Crypto {
public:
    // Bcrypt-like password hashing using PBKDF2
    static std::string hash_password(const std::string& password, int cost = 12) {
        unsigned char salt[16];
        if (RAND_bytes(salt, sizeof(salt)) != 1) {
            throw std::runtime_error("Failed to generate salt");
        }

        unsigned char hash[32];
        int iterations = 1 << cost; // 2^cost iterations

        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                             salt, sizeof(salt),
                             iterations,
                             EVP_sha256(),
                             sizeof(hash), hash) != 1) {
            throw std::runtime_error("Failed to hash password");
        }

        // Format: $pbkdf2$cost$salt$hash
        std::ostringstream oss;
        oss << "$pbkdf2$" << cost << "$";
        oss << to_hex(salt, sizeof(salt)) << "$";
        oss << to_hex(hash, sizeof(hash));

        return oss.str();
    }

    static bool verify_password(const std::string& password, const std::string& hash_str) {
        // Parse hash string
        if (hash_str.substr(0, 8) != "$pbkdf2$") return false;

        size_t pos1 = 8;
        size_t pos2 = hash_str.find('$', pos1);
        if (pos2 == std::string::npos) return false;

        int cost = std::stoi(hash_str.substr(pos1, pos2 - pos1));

        pos1 = pos2 + 1;
        pos2 = hash_str.find('$', pos1);
        if (pos2 == std::string::npos) return false;

        std::string salt_hex = hash_str.substr(pos1, pos2 - pos1);
        std::string hash_hex = hash_str.substr(pos2 + 1);

        unsigned char salt[16];
        unsigned char stored_hash[32];
        from_hex(salt_hex, salt, sizeof(salt));
        from_hex(hash_hex, stored_hash, sizeof(stored_hash));

        // Compute hash of input password
        unsigned char computed_hash[32];
        int iterations = 1 << cost;

        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                             salt, sizeof(salt),
                             iterations,
                             EVP_sha256(),
                             sizeof(computed_hash), computed_hash) != 1) {
            return false;
        }

        // Constant-time comparison
        return CRYPTO_memcmp(computed_hash, stored_hash, sizeof(computed_hash)) == 0;
    }

    static std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.c_str()),
               input.length(), hash);
        return to_hex(hash, SHA256_DIGEST_LENGTH);
    }

    static std::string random_token(size_t bytes = 32) {
        unsigned char buffer[64];
        if (bytes > sizeof(buffer)) bytes = sizeof(buffer);

        if (RAND_bytes(buffer, bytes) != 1) {
            throw std::runtime_error("Failed to generate random token");
        }

        return to_hex(buffer, bytes);
    }

private:
    static std::string to_hex(const unsigned char* data, size_t len) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<int>(data[i]);
        }
        return oss.str();
    }

    static void from_hex(const std::string& hex, unsigned char* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            std::string byte = hex.substr(i * 2, 2);
            data[i] = static_cast<unsigned char>(std::stoi(byte, nullptr, 16));
        }
    }
};

} // namespace forge
