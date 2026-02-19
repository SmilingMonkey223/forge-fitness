# FORGE - Quick Start Guide

Get FORGE up and running in under 5 minutes.

## Prerequisites Check

Before starting, make sure you have:

```bash
# Check C++ compiler (need GCC 11+ or Clang 14+)
g++ --version
# or
clang++ --version

# Check CMake (need 3.20+)
cmake --version

# Check Node.js (need 18+)
node --version

# Check Docker (optional but recommended)
docker --version
docker-compose --version
```

If any are missing, install them:
- **Ubuntu/Debian**: `sudo apt install build-essential cmake nodejs npm docker.io docker-compose`
- **macOS**: `brew install cmake node docker`
- **Arch Linux**: `sudo pacman -S base-devel cmake nodejs npm docker docker-compose`

---

## Option 1: Docker Quick Start (Recommended)

This is the fastest way to get everything running:

```bash
# 1. Clone the repository
git clone <your-repo-url>
cd Fitness

# 2. Set up environment variables
cp .env.example .env

# 3. Edit .env and set a strong JWT_SECRET
nano .env
# Change JWT_SECRET to a random 32+ character string

# 4. Run the magic command
make quickstart

# That's it! The app is now running.
```

**What just happened?**
- Installed frontend dependencies
- Built the C++ backend
- Started PostgreSQL in Docker
- Ran database migrations
- Started backend server on port 8080
- Started frontend dev server on port 5173

**Access the app:**
- Frontend: http://localhost:5173
- Backend API: http://localhost:8080
- Database: localhost:5432 (user: forge, password: forge, db: forge)

---

## Option 2: Manual Setup (For Development)

If you want more control or are developing on the project:

### Step 1: Environment Setup

```bash
cd Fitness
cp .env.example .env
```

Edit `.env` and configure:
```bash
DATABASE_URL=postgresql://forge:forge@localhost:5432/forge
JWT_SECRET=<generate-a-random-32-char-string>
PORT=8080
USDA_API_KEY=<optional-get-from-usda-website>
```

### Step 2: Database Setup

```bash
# Start PostgreSQL with Docker
docker-compose up -d db

# Wait for it to be ready
sleep 5

# Run migrations
make migrate
```

### Step 3: Backend Setup

```bash
# Build the C++ backend
make build-backend

# Start the backend server
cd backend/build
./forge
```

Keep this terminal open. The backend is now running on port 8080.

### Step 4: Frontend Setup

Open a **new terminal**:

```bash
# Install dependencies
cd frontend
npm install

# Start dev server
npm run dev
```

Frontend is now running on port 5173.

### Step 5: Open the App

Visit http://localhost:5173 in your browser.

---

## First Use

### 1. Create an Account

- Click "Sign up" on the login page
- Fill in your details:
  - **Email**: Your email address
  - **Username**: 3-24 characters, letters/numbers/underscores
  - **Display Name**: Your name
  - **Password**: At least 8 characters with uppercase, lowercase, and a digit

### 2. Complete Onboarding (Coming Soon)

Right now, you'll be taken directly to the dashboard after registration. The onboarding wizard (to set your height, weight, goals, etc.) is **coming soon** in the next development phase.

### 3. Explore the Dashboard

You'll see:
- **Macro rings**: Shows your daily nutrition progress (currently showing zeros as no data is logged yet)
- **Workout section**: Placeholder for today's workout
- **Week overview**: Shows which days you've worked out

---

## Development Workflow

### Making Changes

**Backend (C++):**
```bash
# Make your changes to files in backend/src/ or backend/include/

# Rebuild
make build-backend

# Restart the backend
cd backend/build
./forge
```

**Frontend (React):**
```bash
# Make your changes to files in frontend/src/

# Vite will auto-reload - just save and see changes instantly!
```

### Running Tests

```bash
# Backend tests
make test-backend

# Frontend tests
cd frontend && npm test
```

### Checking Code Quality

```bash
# Format and lint everything
make lint

# Or individually
make lint-backend  # C++ formatting
make lint-frontend # TypeScript/React linting
```

### Creating a Database Migration

```bash
./scripts/create-migration.sh add_user_preferences

# Edit the created file in backend/migrations/
# Then apply it:
make migrate
```

---

## Common Commands

```bash
make help          # Show all available commands
make dev           # Start both backend and frontend
make build         # Build everything
make test          # Run all tests
make clean         # Clean build artifacts
make docker-up     # Start all services with Docker
make docker-down   # Stop Docker services
make migrate       # Run database migrations
```

---

## Troubleshooting

### "Cannot connect to database"

**Problem**: Backend can't reach PostgreSQL.

**Solution**:
```bash
# Make sure PostgreSQL is running
docker-compose ps

# If not, start it
docker-compose up -d db

# Check DATABASE_URL in .env matches the Docker config
cat .env | grep DATABASE_URL
# Should be: postgresql://forge:forge@localhost:5432/forge
```

### "Port 8080 already in use"

**Problem**: Another service is using port 8080.

**Solution**:
```bash
# Find what's using the port
lsof -i :8080
# or
netstat -tuln | grep 8080

# Kill it or change PORT in .env
```

### "CMake can't find dependencies"

**Problem**: Missing C++ libraries.

**Solution**:
```bash
# Install missing packages
# Ubuntu/Debian:
sudo apt install libpq-dev libpqxx-dev libssl-dev

# macOS:
brew install libpq libpqxx openssl

# CMake will fetch Crow, jwt-cpp, and json automatically
```

### "npm install fails"

**Problem**: Node version too old or npm cache corrupted.

**Solution**:
```bash
# Check Node version
node --version  # Should be 18+

# Clear npm cache
cd frontend
rm -rf node_modules package-lock.json
npm cache clean --force
npm install
```

### "Migration already applied" error

**Problem**: Trying to run migrations twice.

**Solution**:
```bash
# Check which migrations are applied
psql $DATABASE_URL -c "SELECT * FROM schema_migrations;"

# If you need to reset (⚠️ DELETES ALL DATA):
psql $DATABASE_URL -c "DROP SCHEMA public CASCADE; CREATE SCHEMA public;"
make migrate
```

### Frontend shows "Failed to fetch"

**Problem**: Frontend can't reach backend API.

**Solution**:
```bash
# 1. Make sure backend is running
curl http://localhost:8080/health
# Should return: {"status":"ok","timestamp":...}

# 2. Check CORS settings in backend
# Make sure CORS_ORIGINS in .env includes http://localhost:5173

# 3. Check frontend proxy in vite.config.ts
# Should have proxy: { '/api': { target: 'http://localhost:8080' } }
```

---

## Next Steps

Now that you have FORGE running:

1. **Explore the codebase**
   - Start with `backend/src/main.cpp` to understand the backend
   - Look at `frontend/src/App.tsx` for frontend routing
   - Check `PROJECT_STATUS.md` to see what's done and what's next

2. **Pick a feature to implement**
   - See `PROJECT_STATUS.md` for "In Progress" and "Not Yet Started" sections
   - Good starter tasks:
     - Complete the onboarding wizard UI
     - Implement TDEE calculation
     - Add workout creation endpoint

3. **Read the contributing guide**
   - See `CONTRIBUTING.md` for detailed development guidelines
   - Understand the architecture
   - Learn the testing requirements

4. **Join the development**
   - Fork the repository
   - Pick an issue or feature
   - Submit a pull request

---

## Getting Help

- **Documentation**: Check `README.md`, `PROJECT_STATUS.md`, `CONTRIBUTING.md`
- **Issues**: Search existing issues or create a new one
- **Code Questions**: Read the inline comments in the source code

---

**Happy coding! 🏋️**

Build something amazing with FORGE.
