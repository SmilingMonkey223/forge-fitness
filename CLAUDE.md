# FORGE - Claude Development Guide

## Project Overview

FORGE is a comprehensive fitness tracking application built with C++ backend and React frontend. The goal is to create a production-ready app rivaling Strava, MacroFactor, and Hevy for workout and nutrition tracking.

**Current Status**: Phase 1 Foundation (~35% complete)

## Core Development Workflow

### ⚠️ ALWAYS CREATE PULL REQUESTS

**CRITICAL RULE**: Never push directly to `main`. Always create feature branches and PRs for review.

**Why?**
- Enables code review with CodeRabbit AI
- Maintains clean git history
- Allows for discussion and iteration
- Prevents breaking changes from reaching main

### Standard PR Workflow

1. **Create Feature Branch**
   ```bash
   git checkout -b feat/feature-name
   # or
   git checkout -b fix/bug-name
   ```

2. **Make Changes**
   - Implement your feature
   - Follow code style guidelines (see CONTRIBUTING.md)
   - Add tests for new functionality
   - Update documentation as needed

3. **Commit with Conventional Commits**
   ```bash
   git add .
   git commit -m "feat: add workout PR detection algorithm"
   # Always include Co-Authored-By line:
   git commit --amend
   # Add to commit message:
   # Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
   ```

   Commit types:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation only
   - `test:` - Adding tests
   - `refactor:` - Code change that neither fixes bug nor adds feature
   - `perf:` - Performance improvement
   - `chore:` - Maintenance tasks

4. **Push and Create PR**
   ```bash
   git push -u origin feat/feature-name

   # Then create PR with gh CLI or GitHub UI
   gh pr create --title "feat: add workout PR detection" \
                --body "## Summary
   - Implements Epley formula for 1RM calculation
   - Detects PRs by exercise
   - Adds animated celebration on PR achievement

   ## Test Plan
   - [ ] Unit tests for 1RM calculation
   - [ ] Integration test for PR detection
   - [ ] Manual test with real workout data"
   ```

5. **Wait for CodeRabbit Review**
   - CodeRabbit will automatically review your PR
   - Address any feedback
   - Push updates to the same branch

6. **Merge After Approval**
   - Use "Squash and merge" for clean history
   - Delete branch after merge

### Example PR Creation (Claude)

When implementing a feature:

```bash
# 1. Create branch
git checkout -b feat/user-onboarding

# 2. Make changes (multiple commits OK)
git commit -m "feat: add TDEE calculation service"
git commit -m "feat: add onboarding wizard UI"
git commit -m "test: add TDEE calculation tests"

# 3. Push and create PR
git push -u origin feat/user-onboarding

# 4. Create PR with gh CLI
gh pr create \
  --title "feat: implement user onboarding with TDEE calculation" \
  --body "## Summary
Implements complete onboarding flow for new users.

### Features
- Multi-step wizard (5 screens)
- TDEE calculation using Mifflin-St Jeor equation
- Macro target calculation based on fitness goals
- Form validation with inline errors
- Progress indicator

### Test Plan
- [x] Unit tests for TDEE calculation
- [x] Verify calculation against online calculators
- [x] Test all goal types (lose_fat, maintain, build_muscle)
- [x] Manual test through full onboarding flow

### Performance
- Onboarding completable in under 45 seconds ✓
- All API calls < 200ms ✓

See PROJECT_STATUS.md - completes User Profile & Onboarding task.

🤖 Generated with Claude Code"
```

## Project Architecture

### Backend (C++20)

**Tech Stack:**
- Crow HTTP framework
- PostgreSQL (libpqxx)
- JWT authentication (jwt-cpp)
- OpenSSL for crypto
- Google Test for testing

**Structure:**
```
backend/
├── src/
│   ├── main.cpp              # Server entry point
│   ├── services/             # Business logic
│   │   └── auth_service.cpp
│   ├── controllers/          # HTTP handlers (to be added)
│   ├── models/               # Data models (to be added)
│   └── middleware/           # Auth, CORS, etc. (to be added)
├── include/                  # Header files
│   ├── config.hpp           # Environment config
│   ├── database.hpp         # Connection pool
│   ├── crypto.hpp           # Password hashing, SHA-256
│   ├── jwt.hpp              # Token creation/validation
│   ├── uuid.hpp             # UUID generation
│   └── models.hpp           # Data structures
├── tests/                    # Test suite
└── migrations/               # Database migrations
```

**Key Patterns:**
- Services handle business logic, return domain objects
- Controllers handle HTTP, call services
- Models are pure data structures with `to_json()` methods
- Always use parameterized queries (no SQL injection)
- Connection pool for database (min 5, max 20)

### Frontend (React 18 + TypeScript)

**Tech Stack:**
- React 18 with TypeScript (strict mode)
- Tailwind CSS for styling
- TanStack Query for server state
- Zustand for client state (rest timer, in-progress workout)
- Framer Motion for animations

**Structure:**
```
frontend/src/
├── pages/                # Route-level components
│   ├── Dashboard.tsx
│   ├── Login.tsx
│   └── Register.tsx
├── components/           # Reusable UI
│   └── ui/              # Design system components
├── services/            # API clients
│   └── api.ts
├── hooks/               # Custom React hooks
├── contexts/            # React contexts
├── types/               # TypeScript types
└── styles/              # Global CSS
```

**Key Patterns:**
- Pages are route components, use hooks for logic
- Extract complex logic into custom hooks
- Use TanStack Query for all API calls
- Keep components small and focused
- Mobile-first responsive design

### Database

**Schema (12 tables):**
- `users` - User accounts
- `user_profiles` - Profile data (height, weight, goals)
- `refresh_tokens` - JWT refresh token storage
- `exercises` - Exercise library (200+ seeded)
- `workouts` - Workout sessions
- `exercise_sets` - Individual sets in workouts
- `routines` - Workout templates
- `routine_exercises` - Exercises in templates
- `routine_sets` - Set templates
- `nutrition_log` - Food entries
- `custom_foods` - User-created foods
- `usda_food_cache` - USDA API cache

**Migration Pattern:**
```sql
-- Always start with version number and description
-- Migration: add_user_preferences

-- Your changes here

-- Always end by recording the migration
INSERT INTO schema_migrations (version) VALUES (8);
```

## Current Implementation Status

### ✅ Completed
- Project structure and build system
- Docker Compose setup
- Database schema (12 tables, 200+ exercises)
- Authentication system (register, login, JWT)
- Basic frontend (login, register, dashboard UI)
- Development tooling (Makefile, scripts)
- CI/CD pipeline (GitHub Actions)

### 🚧 Next Priority (In Order)

1. **User Profile & Onboarding**
   - [ ] Profile CRUD endpoints
   - [ ] TDEE calculation (Mifflin-St Jeor)
   - [ ] Macro target calculation
   - [ ] Onboarding wizard UI

2. **Dashboard Aggregation**
   - [ ] Real data from database
   - [ ] Caching layer (< 200ms p95)
   - [ ] Weekly trends calculation

3. **Workout Tracking**
   - [ ] Workout CRUD endpoints
   - [ ] Exercise set logging
   - [ ] PR detection (Epley formula)
   - [ ] Previous performance retrieval
   - [ ] Active workout UI
   - [ ] Rest timer

4. **Nutrition Tracking**
   - [ ] USDA API integration
   - [ ] Food search with caching
   - [ ] Nutrition log CRUD
   - [ ] Daily summary aggregation
   - [ ] Food logging UI

See `PROJECT_STATUS.md` for detailed feature tracking.

## Performance Requirements

All PRs must meet these budgets:

| Metric | Target | How to Check |
|--------|--------|--------------|
| Dashboard API | < 200ms p95 | Server logs |
| Workout write | < 100ms p95 | Server logs |
| Food search (cached) | < 300ms | Server logs |
| Frontend bundle | < 500KB gzipped | `npm run build` output |
| Initial load | < 2s on 4G | Lighthouse |

Run `make test` before creating PR to verify performance.

## Testing Requirements

**Minimum for PR approval:**
- Unit tests for new services (≥80% coverage)
- Integration test for new endpoints
- Manual testing completed
- All existing tests pass

**Test Structure:**
```cpp
// backend/tests/unit/tdee_calculator_test.cpp
#include <gtest/gtest.h>
#include "profile_service.hpp"

TEST(TDEECalculatorTest, MaleCalculation) {
    ProfileService::TDEERequest req{
        .sex = "male",
        .weight_kg = 80.0,
        .height_cm = 180.0,
        .age = 30,
        .activity_level = "moderately_active"
    };

    double tdee = ProfileService::calculate_tdee(req);
    EXPECT_NEAR(tdee, 2700, 50); // Allow 50 cal variance
}
```

## Common Commands

```bash
# Development
make dev              # Start backend + frontend
make build            # Build everything
make test             # Run all tests
make migrate          # Run migrations

# Docker
make docker-up        # Start full stack
make docker-down      # Stop services

# Code Quality
make lint             # Format and lint
make clean            # Clean build artifacts

# Git
git checkout -b feat/feature-name
git commit -m "feat: description"
git push -u origin feat/feature-name
gh pr create
```

## Key Files to Know

- `PROJECT_STATUS.md` - Feature tracking, what's done and what's next
- `CONTRIBUTING.md` - Detailed development guidelines
- `QUICKSTART.md` - Getting started guide
- `forge phase1.md` - Original specification
- `backend/src/main.cpp` - Server entry point
- `frontend/src/App.tsx` - Frontend routing

## Code Review Checklist

Before creating PR, verify:

- [ ] Follows conventional commits
- [ ] Tests added and passing
- [ ] Documentation updated
- [ ] No console.logs or debug code
- [ ] Performance budgets met
- [ ] Mobile responsive (if UI change)
- [ ] Error handling implemented
- [ ] TypeScript strict mode passes
- [ ] C++ compiles with -Werror
- [ ] Matches existing code style

## Security Guidelines

- Never commit `.env` files or secrets
- Always use parameterized queries
- Validate all user input
- Hash passwords with PBKDF2 (cost 12)
- Use constant-time comparison for passwords
- Set secure HTTP headers (CORS, CSP, etc.)
- Follow principle of least privilege

## Getting Help

- Check `PROJECT_STATUS.md` for current work
- Read `CONTRIBUTING.md` for detailed guidelines
- Review existing code for patterns
- Ask questions in PR comments

## Phase 1 Definition of Done

Phase 1 is complete when all items in `PROJECT_STATUS.md` under "Definition of Done" are checked. Current completion: ~35%.

---

**Remember**: Always create PRs, never push to main! CodeRabbit is watching. 🤖
