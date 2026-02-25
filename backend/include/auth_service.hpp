#pragma once

#include "models.hpp"
#include "database.hpp"
#include "crypto.hpp"
#include "jwt.hpp"
#include "uuid.hpp"
#include <regex>
#include <algorithm>
#include <cctype>

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

    static bool validate_email(const std::string& email) {
        static const std::regex pattern(
            R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
        );
        return std::regex_match(email, pattern);
    }

    static bool validate_username(const std::string& username) {
        if (username.length() < 3 || username.length() > 24) return false;

        return std::all_of(username.begin(), username.end(), [](char c) {
            return std::isalnum(c) || c == '_';
        });
    }

    static bool validate_password(const std::string& password) {
        if (password.length() < 8 || password.length() > 128) return false;

        bool has_upper = false, has_lower = false, has_digit = false;

        for (char c : password) {
            if (std::isupper(c)) has_upper = true;
            if (std::islower(c)) has_lower = true;
            if (std::isdigit(c)) has_digit = true;
        }

        return has_upper && has_lower && has_digit;
    }

    static std::pair<User, JWT::TokenPair> register_user(const RegisterRequest& req) {
        if (!validate_email(req.email)) {
            throw std::invalid_argument("INVALID_EMAIL");
        }

        if (!validate_username(req.username)) {
            throw std::invalid_argument("INVALID_USERNAME");
        }

        if (!validate_password(req.password)) {
            throw std::invalid_argument("WEAK_PASSWORD");
        }

        auto conn = Database::instance().get_connection();
        pqxx::work txn(*conn);

        auto email_result = txn.exec_params(
            "SELECT id FROM users WHERE LOWER(email) = LOWER($1) AND deleted_at IS NULL",
            req.email
        );

        if (!email_result.empty()) {
            throw std::invalid_argument("EMAIL_TAKEN");
        }

        auto username_result = txn.exec_params(
            "SELECT id FROM users WHERE LOWER(username) = LOWER($1) AND deleted_at IS NULL",
            req.username
        );

        if (!username_result.empty()) {
            throw std::invalid_argument("USERNAME_TAKEN");
        }

        std::string password_hash = Crypto::hash_password(req.password, 12);
        std::string user_id = UUID::generate();

        txn.exec_params(
            "INSERT INTO users (id, email, username, display_name, password_hash) "
            "VALUES ($1, $2, $3, $4, $5)",
            user_id, req.email, req.username, req.display_name, password_hash
        );

        auto tokens = JWT::create_token_pair(user_id, req.username);

        std::string token_hash = Crypto::sha256(tokens.refresh_token);
        auto expires_at = std::chrono::system_clock::now() + std::chrono::hours(24 * 30);

        txn.exec_params(
            "INSERT INTO refresh_tokens (user_id, token_hash, expires_at) "
            "VALUES ($1, $2, $3)",
            user_id, token_hash,
            std::chrono::system_clock::to_time_t(expires_at)
        );

        txn.commit();

        User user;
        user.id = user_id;
        user.email = req.email;
        user.username = req.username;
        user.display_name = req.display_name;

        return {user, tokens};
    }

    static std::pair<User, JWT::TokenPair> login(const LoginRequest& req) {
        auto conn = Database::instance().get_connection();
        pqxx::work txn(*conn);

        auto result = txn.exec_params(
            "SELECT id, email, username, display_name, password_hash "
            "FROM users WHERE LOWER(email) = LOWER($1) AND deleted_at IS NULL",
            req.email
        );

        if (result.empty()) {
            throw std::invalid_argument("INVALID_CREDENTIALS");
        }

        auto row = result[0];
        std::string user_id = row["id"].c_str();
        std::string email = row["email"].c_str();
        std::string username = row["username"].c_str();
        std::string display_name = row["display_name"].c_str();
        std::string password_hash = row["password_hash"].c_str();

        if (!Crypto::verify_password(req.password, password_hash)) {
            throw std::invalid_argument("INVALID_CREDENTIALS");
        }

        auto tokens = JWT::create_token_pair(user_id, username);

        std::string token_hash = Crypto::sha256(tokens.refresh_token);
        auto expires_at = std::chrono::system_clock::now() + std::chrono::hours(24 * 30);

        txn.exec_params(
            "INSERT INTO refresh_tokens (user_id, token_hash, expires_at) "
            "VALUES ($1, $2, $3)",
            user_id, token_hash,
            std::chrono::system_clock::to_time_t(expires_at)
        );

        txn.commit();

        User user;
        user.id = user_id;
        user.email = email;
        user.username = username;
        user.display_name = display_name;

        return {user, tokens};
    }

    static JWT::TokenPair refresh(const std::string& refresh_token) {
        auto decoded = JWT::verify(refresh_token);
        std::string token_hash = Crypto::sha256(refresh_token);

        auto conn = Database::instance().get_connection();
        pqxx::work txn(*conn);

        auto result = txn.exec_params(
            "SELECT user_id, expires_at FROM refresh_tokens "
            "WHERE token_hash = $1 AND revoked_at IS NULL",
            token_hash
        );

        if (result.empty()) {
            throw std::invalid_argument("INVALID_TOKEN");
        }

        auto row = result[0];
        std::string user_id = row["user_id"].c_str();

        txn.exec_params(
            "UPDATE refresh_tokens SET revoked_at = CURRENT_TIMESTAMP "
            "WHERE token_hash = $1",
            token_hash
        );

        auto user_result = txn.exec_params(
            "SELECT username FROM users WHERE id = $1",
            user_id
        );

        if (user_result.empty()) {
            throw std::invalid_argument("USER_NOT_FOUND");
        }

        std::string username = user_result[0]["username"].c_str();

        auto tokens = JWT::create_token_pair(user_id, username);

        std::string new_token_hash = Crypto::sha256(tokens.refresh_token);
        auto expires_at = std::chrono::system_clock::now() + std::chrono::hours(24 * 30);

        txn.exec_params(
            "INSERT INTO refresh_tokens (user_id, token_hash, expires_at) "
            "VALUES ($1, $2, $3)",
            user_id, new_token_hash,
            std::chrono::system_clock::to_time_t(expires_at)
        );

        txn.commit();

        return tokens;
    }
};

} // namespace forge
