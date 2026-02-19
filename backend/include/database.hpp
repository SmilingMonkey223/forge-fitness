#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>

namespace forge {

class ConnectionPool {
public:
    explicit ConnectionPool(const std::string& connection_string,
                          size_t min_size = 5,
                          size_t max_size = 20,
                          std::chrono::seconds timeout = std::chrono::seconds(5))
        : connection_string_(connection_string)
        , min_size_(min_size)
        , max_size_(max_size)
        , timeout_(timeout)
        , current_size_(0) {
        for (size_t i = 0; i < min_size_; ++i) {
            connections_.push(create_connection());
            ++current_size_;
        }
    }

    ~ConnectionPool() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!connections_.empty()) {
            connections_.pop();
        }
    }

    class Connection {
    public:
        Connection(std::unique_ptr<pqxx::connection> conn, ConnectionPool* pool)
            : conn_(std::move(conn)), pool_(pool) {}

        ~Connection() {
            if (conn_ && pool_) {
                pool_->return_connection(std::move(conn_));
            }
        }

        Connection(Connection&& other) noexcept
            : conn_(std::move(other.conn_)), pool_(other.pool_) {
            other.pool_ = nullptr;
        }

        Connection& operator=(Connection&& other) noexcept {
            if (this != &other) {
                conn_ = std::move(other.conn_);
                pool_ = other.pool_;
                other.pool_ = nullptr;
            }
            return *this;
        }

        pqxx::connection* operator->() { return conn_.get(); }
        pqxx::connection& operator*() { return *conn_; }

    private:
        std::unique_ptr<pqxx::connection> conn_;
        ConnectionPool* pool_;
    };

    Connection acquire() {
        std::unique_lock<std::mutex> lock(mutex_);

        if (!condition_.wait_for(lock, timeout_, [this] {
            return !connections_.empty() || current_size_ < max_size_;
        })) {
            throw std::runtime_error("Connection pool timeout");
        }

        if (connections_.empty()) {
            ++current_size_;
            return Connection(create_connection(), this);
        }

        auto conn = std::move(connections_.front());
        connections_.pop();
        return Connection(std::move(conn), this);
    }

private:
    std::unique_ptr<pqxx::connection> create_connection() {
        return std::make_unique<pqxx::connection>(connection_string_);
    }

    void return_connection(std::unique_ptr<pqxx::connection> conn) {
        std::unique_lock<std::mutex> lock(mutex_);
        connections_.push(std::move(conn));
        condition_.notify_one();
    }

    std::string connection_string_;
    size_t min_size_;
    size_t max_size_;
    std::chrono::seconds timeout_;
    size_t current_size_;
    std::queue<std::unique_ptr<pqxx::connection>> connections_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

class Database {
public:
    static Database& instance() {
        static Database db;
        return db;
    }

    ConnectionPool::Connection get_connection() {
        return pool_->acquire();
    }

    void initialize(const std::string& connection_string) {
        pool_ = std::make_unique<ConnectionPool>(connection_string);
    }

private:
    Database() = default;
    std::unique_ptr<ConnectionPool> pool_;
};

} // namespace forge
