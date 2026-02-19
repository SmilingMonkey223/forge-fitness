#pragma once

#include <string>
#include <jwt-cpp/jwt.h>
#include <chrono>
#include "config.hpp"

namespace forge {

class JWT {
public:
    struct TokenPair {
        std::string access_token;
        std::string refresh_token;
    };

    static std::string create_access_token(const std::string& user_id,
                                          const std::string& username) {
        auto now = std::chrono::system_clock::now();
        auto exp = now + std::chrono::minutes(15);

        return jwt::create()
            .set_issuer("forge")
            .set_type("JWT")
            .set_issued_at(now)
            .set_expires_at(exp)
            .set_payload_claim("user_id", jwt::claim(user_id))
            .set_payload_claim("username", jwt::claim(username))
            .sign(jwt::algorithm::hs256{Config::instance().jwt_secret()});
    }

    static std::string create_refresh_token() {
        auto now = std::chrono::system_clock::now();
        auto exp = now + std::chrono::hours(24 * 30); // 30 days

        return jwt::create()
            .set_issuer("forge")
            .set_type("JWT")
            .set_issued_at(now)
            .set_expires_at(exp)
            .sign(jwt::algorithm::hs256{Config::instance().jwt_secret()});
    }

    static TokenPair create_token_pair(const std::string& user_id,
                                       const std::string& username) {
        return {
            create_access_token(user_id, username),
            create_refresh_token()
        };
    }

    static jwt::decoded_jwt<jwt::traits::kazuho_picojson> verify(const std::string& token) {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{Config::instance().jwt_secret()})
            .with_issuer("forge");

        auto decoded = jwt::decode(token);
        verifier.verify(decoded);
        return decoded;
    }

    static std::string get_user_id(const jwt::decoded_jwt<jwt::traits::kazuho_picojson>& token) {
        return token.get_payload_claim("user_id").as_string();
    }

    static std::string get_username(const jwt::decoded_jwt<jwt::traits::kazuho_picojson>& token) {
        return token.get_payload_claim("username").as_string();
    }
};

} // namespace forge
