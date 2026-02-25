#pragma once

#include "models.hpp"
#include "jwt.hpp"
#include <string>
#include <utility>

namespace forge {

class AuthService {
public:
    struct RegisterRequest {
        std::string email;
        std::string username;
        std::string password;
        std::string display_name;
    };

    struct LoginRequest {
        std::string email;
        std::string password;
    };

    enum class ErrorCode {
        EMAIL_TAKEN,
        USERNAME_TAKEN,
        WEAK_PASSWORD,
        INVALID_EMAIL,
        INVALID_CREDENTIALS,
        RATE_LIMITED
    };

    static bool validate_email(const std::string& email);
    static bool validate_username(const std::string& username);
    static bool validate_password(const std::string& password);

    static std::pair<User, JWT::TokenPair> register_user(const RegisterRequest& req);
    static std::pair<User, JWT::TokenPair> login(const LoginRequest& req);
    static JWT::TokenPair refresh(const std::string& refresh_token);
};

} // namespace forge
