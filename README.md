# FORGE - Phase 1: Foundation

A comprehensive fitness tracking application built with C++ backend and React frontend.

## Features

- **Authentication**: Secure JWT-based authentication with bcrypt password hashing
- **Workout Tracking**: Log exercises, sets, reps, weight, and RPE with PR detection
- **Nutrition Tracking**: Log meals with USDA FoodData Central integration
- **Dashboard**: Real-time progress tracking with macro targets and workout summaries
- **Profile Management**: Personalized TDEE and macro calculations

## Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 11+, Clang 14+, or MSVC 19.29+)
- PostgreSQL 16+
- Node.js 18+
- Docker and Docker Compose (for containerized deployment)

## Quick Start

### Using Docker Compose (Recommended)

```bash
# Copy environment variables
cp .env.example .env

# Edit .env with your configuration
nano .env

# Start the entire stack
docker-compose up
```

The application will be available at:
- Frontend: http://localhost:5173
- Backend API: http://localhost:8080

### Manual Setup

#### Backend

```bash
cd backend

# Install dependencies (using vcpkg)
vcpkg install crow libpqxx nlohmann-json openssl bcrypt jwt-cpp

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run migrations
./forge migrate

# Start server
./forge serve
```

#### Frontend

```bash
cd frontend

# Install dependencies
npm install

# Start dev server
npm run dev
```

## Development

### Running Tests

```bash
# Backend tests
cd backend/build
cmake --build . --target test

# Frontend tests
cd frontend
npm test
```

### Code Formatting

```bash
# Backend (C++)
make lint

# Frontend (TypeScript/React)
cd frontend
npm run lint
```

### Database Migrations

```bash
# Apply pending migrations
make migrate

# Create new migration
./scripts/create-migration.sh <migration_name>
```

## Project Structure

```
forge/
├── backend/
│   ├── src/
│   │   ├── controllers/    # HTTP request handlers
│   │   ├── services/       # Business logic
│   │   ├── models/         # Data models
│   │   ├── middleware/     # Auth, CORS, logging
│   │   ├── utils/          # Helpers and utilities
│   │   └── config/         # Configuration management
│   ├── tests/
│   │   ├── unit/           # Unit tests
│   │   └── integration/    # Integration tests
│   ├── migrations/         # Database migrations
│   └── CMakeLists.txt
├── frontend/
│   ├── src/
│   │   ├── components/     # React components
│   │   ├── pages/          # Page components
│   │   ├── services/       # API clients
│   │   ├── hooks/          # Custom React hooks
│   │   └── contexts/       # React contexts
│   └── package.json
└── docker-compose.yml
```

## API Documentation

API documentation is available at `/api/docs` when running the backend server.

Key endpoints:
- `POST /api/auth/register` - User registration
- `POST /api/auth/login` - User login
- `GET /api/dashboard` - Dashboard data
- `POST /api/workouts` - Create workout
- `POST /api/nutrition/log` - Log food

## Performance Budgets

- Dashboard API: < 200ms p95
- Workout log write: < 100ms p95
- Food search (cached): < 300ms p95
- Frontend initial load: < 2s on 4G
- Frontend bundle size: < 500KB gzipped

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

MIT License - see LICENSE file for details
