#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>

namespace forge {

class Config {
public:
    static Config& instance() {
        static Config config;
        return config;
    }

    std::string database_url() const { return database_url_; }
    std::string jwt_secret() const { return jwt_secret_; }
    int port() const { return port_; }
    std::string log_level() const { return log_level_; }
    std::string cors_origins() const { return cors_origins_; }
    std::string usda_api_key() const { return usda_api_key_; }

private:
    Config() {
        database_url_ = get_env("DATABASE_URL", "");
        jwt_secret_ = get_env("JWT_SECRET", "");
        port_ = std::stoi(get_env("PORT", "8080"));
        log_level_ = get_env("LOG_LEVEL", "info");
        cors_origins_ = get_env("CORS_ORIGINS", "http://localhost:5173");
        usda_api_key_ = get_env("USDA_API_KEY", "");

        if (database_url_.empty()) {
            throw std::runtime_error("DATABASE_URL environment variable is required");
        }
        if (jwt_secret_.empty() || jwt_secret_.length() < 32) {
            throw std::runtime_error("JWT_SECRET must be at least 32 characters");
        }
    }

    std::string get_env(const char* name, const char* default_value) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : std::string(default_value);
    }

    std::string database_url_;
    std::string jwt_secret_;
    int port_;
    std::string log_level_;
    std::string cors_origins_;
    std::string usda_api_key_;
};

} // namespace forge
