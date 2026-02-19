# Contributing to FORGE

Thank you for your interest in contributing to FORGE! This document provides guidelines and instructions for contributing.

## Development Setup

### Prerequisites

- C++20 compatible compiler (GCC 11+, Clang 14+)
- CMake 3.20+
- PostgreSQL 16+
- Node.js 18+
- Docker and Docker Compose (recommended)

### Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd Fitness

# Quick setup (recommended for first-time)
make quickstart

# Or manual setup
cp .env.example .env
make install
make build
make docker-up

# Start development
make dev
```

## Development Workflow

### Backend Development

The backend is written in C++20 using the Crow HTTP framework.

```bash
# Build backend
make build-backend

# Run backend tests
make test-backend

# Format code
make lint-backend
```

Key files:
- `backend/src/main.cpp` - Main server entry point
- `backend/src/services/` - Business logic layer
- `backend/src/controllers/` - HTTP request handlers
- `backend/include/` - Header files

### Frontend Development

The frontend is built with React 18, TypeScript, and Tailwind CSS.

```bash
# Install dependencies
cd frontend && npm install

# Start dev server
npm run dev

# Run tests
npm test

# Lint and format
npm run lint
```

Key files:
- `frontend/src/pages/` - Page components
- `frontend/src/components/` - Reusable components
- `frontend/src/services/api.ts` - API client
- `frontend/src/types/` - TypeScript types

### Database Migrations

Migrations are SQL files in `backend/migrations/` numbered sequentially.

```bash
# Create a new migration
./scripts/create-migration.sh add_new_table

# Run migrations
make migrate
```

## Code Style

### C++

- Follow LLVM coding style
- Use `clang-format` for formatting (run `make lint-backend`)
- Maximum line width: 100 characters
- Use modern C++20 features where appropriate
- All warnings treated as errors (`-Werror`)

### TypeScript/React

- Use TypeScript strict mode
- Follow React hooks best practices
- Use Tailwind utility classes for styling
- Run ESLint before committing (`npm run lint`)

## Testing

### Backend Tests

- Write unit tests for all services and models
- Use Google Test framework
- Aim for ≥80% code coverage
- Test files go in `backend/tests/`

```cpp
#include <gtest/gtest.h>
#include "auth_service.hpp"

TEST(AuthServiceTest, ValidateEmailCorrectly) {
    EXPECT_TRUE(AuthService::validate_email("user@example.com"));
    EXPECT_FALSE(AuthService::validate_email("invalid-email"));
}
```

### Frontend Tests

- Write tests for complex components and hooks
- Use Vitest for testing
- Test files: `*.test.tsx` or `*.test.ts`

## Performance Requirements

All contributions must meet these performance budgets:

- Dashboard API: < 200ms p95
- Workout log write: < 100ms p95
- Frontend bundle size: < 500KB gzipped
- Frontend initial load: < 2s on 4G

Run benchmarks before submitting:
```bash
# Backend benchmarks
cd backend/build && ./forge_bench

# Frontend bundle analysis
cd frontend && npm run build -- --analyze
```

## Commit Guidelines

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: add workout PR detection algorithm
fix: resolve JWT token expiry bug
docs: update API documentation
test: add nutrition service tests
perf: optimize dashboard query
refactor: simplify auth middleware
```

## Pull Request Process

1. **Fork & Branch**: Create a feature branch from `main`
   ```bash
   git checkout -b feat/amazing-feature
   ```

2. **Develop**: Make your changes following code style guidelines

3. **Test**: Ensure all tests pass
   ```bash
   make test
   ```

4. **Commit**: Use conventional commit messages

5. **Push**: Push to your fork
   ```bash
   git push origin feat/amazing-feature
   ```

6. **PR**: Open a pull request with:
   - Clear description of changes
   - Link to related issues
   - Screenshots for UI changes
   - Performance impact notes

7. **Review**: Address review feedback

8. **Merge**: Maintainer will merge once approved

## CI/CD Pipeline

All PRs must pass:
- ✓ Backend build and tests
- ✓ Frontend build and lint
- ✓ Code formatting checks
- ✓ Bundle size checks

GitHub Actions runs automatically on each push.

## Architecture Guidelines

### Backend

- **Services**: Pure business logic, no HTTP concerns
- **Controllers**: Handle HTTP requests/responses, call services
- **Models**: Data structures and serialization
- **Middleware**: Cross-cutting concerns (auth, CORS, logging)

### Frontend

- **Pages**: Route-level components
- **Components**: Reusable UI components
- **Services**: API clients and external integrations
- **Hooks**: Custom React hooks for shared logic
- **Contexts**: Global state (avoid overuse, prefer React Query)

### Database

- Always use migrations for schema changes
- Index foreign keys and frequently queried columns
- Use UUIDs for primary keys
- Soft delete pattern (`deleted_at` timestamp)

## Security

- Never commit secrets or API keys
- Always use parameterized queries (no SQL injection)
- Validate all user input
- Use constant-time comparison for passwords
- Follow OWASP Top 10 guidelines

Report security vulnerabilities privately to the maintainers.

## Questions?

- Open an issue for bugs or feature requests
- Start a discussion for questions
- Check existing issues/discussions first

Happy coding! 🏋️
