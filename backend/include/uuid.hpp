#pragma once

#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace forge {

class UUID {
public:
    static std::string generate() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        uint64_t data[2];
        data[0] = dis(gen);
        data[1] = dis(gen);

        // Set version to 4 (random UUID)
        data[0] = (data[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        // Set variant to RFC 4122
        data[1] = (data[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        // Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
        oss << std::setw(8) << (data[0] >> 32)
            << '-'
            << std::setw(4) << ((data[0] >> 16) & 0xFFFF)
            << '-'
            << std::setw(4) << (data[0] & 0xFFFF)
            << '-'
            << std::setw(4) << (data[1] >> 48)
            << '-'
            << std::setw(12) << (data[1] & 0xFFFFFFFFFFFFULL);

        return oss.str();
    }

    static bool is_valid(const std::string& uuid) {
        if (uuid.length() != 36) return false;
        if (uuid[8] != '-' || uuid[13] != '-' ||
            uuid[18] != '-' || uuid[23] != '-') return false;

        for (size_t i = 0; i < uuid.length(); ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) continue;
            char c = uuid[i];
            if (!((c >= '0' && c <= '9') ||
                  (c >= 'a' && c <= 'f') ||
                  (c >= 'A' && c <= 'F'))) {
                return false;
            }
        }
        return true;
    }
};

} // namespace forge
