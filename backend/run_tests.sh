#!/bin/bash
set -e

echo "=== Building FORGE Tests ==="
cd "$(dirname "$0")"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make forge_tests -j$(nproc)

echo ""
echo "=== Running Unit Tests ==="
./forge_tests --gtest_filter="*Unit*:*Calculation*:*Password*:*JWT*:*UUID*:*Serialization*:*Validation*:*Crypto*"

echo ""
echo "=== Running Integration Tests (requires PostgreSQL) ==="
if [ -n "$DATABASE_URL" ]; then
    ./forge_tests --gtest_filter="*Integration*"
else
    echo "Skipping integration tests - DATABASE_URL not set"
    echo "To run integration tests:"
    echo "  export DATABASE_URL='postgresql://forge:forge@localhost:5432/forge_test'"
    echo "  export JWT_SECRET='your-secret-at-least-32-chars-long'"
    echo "  ./forge_tests --gtest_filter='*Integration*'"
fi

echo ""
echo "=== Done ==="
