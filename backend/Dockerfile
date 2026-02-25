# Multi-stage build for C++ backend

# Stage 1: Build
FROM debian:bookworm-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    libpqxx-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libasio-dev \
    libboost-system-dev \
    libboost-date-time-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source files
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY include/ ./include/
COPY migrations/ ./migrations/

# Build
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime
FROM debian:bookworm-slim

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libpq5 \
    libpqxx-6.4 \
    libssl3 \
    libcurl4 \
    curl \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary, migrations, and migration runner from builder
COPY --from=builder /app/build/forge ./
COPY --from=builder /app/migrations ./migrations/
COPY run_migrations.sh ./
RUN chmod +x run_migrations.sh

# Non-root user
RUN useradd -m -u 1000 forge && chown -R forge:forge /app
USER forge

EXPOSE 8080

CMD ["./forge"]
