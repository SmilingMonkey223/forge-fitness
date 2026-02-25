.PHONY: help dev build test clean migrate docker-up docker-down install lint

# Default target
help:
	@echo "FORGE Development Commands:"
	@echo ""
	@echo "  make install      - Install all dependencies (backend + frontend)"
	@echo "  make dev          - Start development servers (backend + frontend)"
	@echo "  make build        - Build both backend and frontend"
	@echo "  make test         - Run all tests"
	@echo "  make migrate      - Run database migrations"
	@echo "  make docker-up    - Start all services with Docker Compose"
	@echo "  make docker-down  - Stop all Docker services"
	@echo "  make clean        - Clean build artifacts"
	@echo "  make lint         - Run linters on backend and frontend"
	@echo ""

# Install dependencies
install:
	@echo "Installing frontend dependencies..."
	cd frontend && npm install
	@echo "Frontend dependencies installed!"
	@echo ""
	@echo "Note: Backend C++ dependencies are managed via CMake FetchContent"
	@echo "Run 'make build' to compile the backend"

# Development
dev:
	@echo "Starting development environment..."
	@echo "Starting backend and frontend in parallel..."
	@$(MAKE) -j2 dev-backend dev-frontend

dev-backend:
	@echo "Starting backend server..."
	@if [ -f .env ]; then set -a && . ./.env && set +a; fi && cd backend/build && ./forge

dev-frontend:
	@echo "Starting frontend dev server..."
	cd frontend && npm run dev

# Build
build: build-backend build-frontend

build-backend:
	@echo "Building C++ backend..."
	mkdir -p backend/build
	cd backend/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -j$$(nproc)
	@echo "Backend built successfully!"

build-frontend:
	@echo "Building React frontend..."
	cd frontend && npm run build
	@echo "Frontend built successfully!"

# Testing
test: test-backend test-frontend

test-backend:
	@echo "Running backend tests..."
	cd backend/build && ctest --output-on-failure

test-frontend:
	@echo "Running frontend tests..."
	cd frontend && npm test

# Database migrations
migrate:
	@echo "Running database migrations..."
	@if [ -z "$$DATABASE_URL" ]; then \
		echo "Error: DATABASE_URL environment variable not set"; \
		echo "Please set it or use: make docker-up"; \
		exit 1; \
	fi
	@for file in backend/migrations/*.sql; do \
		echo "Applying $$file..."; \
		psql $$DATABASE_URL -f $$file; \
	done
	@echo "Migrations completed!"

# Docker
docker-up:
	@echo "Starting Docker services..."
	docker compose up -d --build
	@echo ""
	@echo "FORGE is ready!"
	@echo "  - Backend API: http://localhost:8080"
	@echo "  - Frontend: http://localhost:5173"
	@echo "  - PostgreSQL: localhost:5432"

docker-down:
	@echo "Stopping Docker services..."
	docker compose down
	@echo "Services stopped!"

docker-clean:
	@echo "Removing Docker volumes and images..."
	docker compose down -v
	@echo "Cleaned!"

# Lint
lint: lint-backend lint-frontend

lint-backend:
	@echo "Running C++ linter..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find backend/src backend/include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i; \
		echo "C++ code formatted!"; \
	else \
		echo "clang-format not found. Skipping."; \
	fi

lint-frontend:
	@echo "Running frontend linter..."
	cd frontend && npm run lint

# Clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf backend/build
	rm -rf frontend/dist
	rm -rf frontend/node_modules/.vite
	@echo "Clean complete!"

# Quick start for first-time setup
quickstart:
	@echo "FORGE Quick Start"
	@echo "================="
	@echo ""
	@echo "Step 1: Copying environment variables..."
	@if [ ! -f .env ]; then \
		cp .env.example .env; \
		echo "Created .env file. Please edit it with your configuration."; \
		echo ""; \
	fi
	@echo "Step 2: Installing dependencies..."
	@$(MAKE) install
	@echo ""
	@echo "Step 3: Building backend..."
	@$(MAKE) build-backend
	@echo ""
	@echo "Step 4: Starting Docker services..."
	@$(MAKE) docker-up
	@echo ""
	@echo "✓ Setup complete!"
	@echo ""
	@echo "To start development:"
	@echo "  make dev"
