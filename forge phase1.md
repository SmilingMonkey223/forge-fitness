# FORGE — Phase 1: Foundation

## Detailed Functional Specification

> **Benchmark apps:** Strava (social/feed), MacroFactor (nutrition tracking), Jeff Nippard Essentials + Hevy (workout tracking)
>
> **Phase 1 goal:** A working app where you can register, log workouts, log meals with macros, and see a dashboard summarizing your day. No social features yet — those are Phase 3. This phase is pure infrastructure and core tracking.

---

## 1. Authentication System

### 1.1 Registration

**What the best apps do:** MacroFactor onboards in under 60 seconds. Strava lets you sign up with Google/Apple and immediately starts a guided setup. Both collect only what's needed upfront and defer the rest.

**Concrete requirements:**

- `POST /api/auth/register` accepts: `email`, `username`, `password`, `display_name`
- Email must be validated with regex pattern `^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$` and checked for uniqueness (case-insensitive)
- Username: 3–24 characters, alphanumeric + underscores only, unique, case-insensitive matching
- Password: minimum 8 characters, at least one uppercase, one lowercase, one digit. No maximum length under 128 characters
- Password hashed with bcrypt, cost factor 12
- On success: return JWT access token (15 min expiry) + refresh token (30 day expiry). Set refresh token as HttpOnly cookie
- On failure: return specific error codes — `EMAIL_TAKEN`, `USERNAME_TAKEN`, `WEAK_PASSWORD`, `INVALID_EMAIL` — not generic 400s
- Response time: registration must complete in under 500ms including DB write and hash computation

### 1.2 Login

- `POST /api/auth/login` accepts `email` + `password` (not username — email is the canonical identifier)
- Bcrypt compare against stored hash
- Rate limit: 5 failed attempts per email per 15-minute window. After 5 failures, return `429 Too Many Requests` with `Retry-After` header
- On success: same token pair as registration
- On failure: always return `INVALID_CREDENTIALS` regardless of whether email exists (prevent user enumeration)

### 1.3 Token Management

- `POST /api/auth/refresh` — accepts refresh token from HttpOnly cookie, returns new access + refresh token pair (rotation)
- Old refresh token invalidated on use (one-time use tokens)
- Access token is a signed JWT containing: `user_id`, `username`, `iat`, `exp`
- JWT signed with HS256 using a 256-bit secret loaded from environment variable `JWT_SECRET`
- All authenticated endpoints validate token signature and expiry. Invalid/expired token → `401 Unauthorized`

### 1.4 C++ Implementation Specifics

- JWT library: either `jwt-cpp` or hand-roll HMAC-SHA256 with OpenSSL (the learning exercise is more valuable)
- Bcrypt: use `libbcrypt` or `bcrypt.h` from OpenBSD
- Store refresh tokens in PostgreSQL table `refresh_tokens(id, user_id, token_hash, expires_at, revoked_at)`
- Never store raw refresh tokens — store SHA-256 hash only

---

## 2. User Profile & Onboarding

### 2.1 Profile Data Model

**What the best apps do:** MacroFactor collects height, weight, DOB, sex, activity level, and goal during onboarding — all used for TDEE calculation. Strava collects height, weight, and FTP for athletes. Hevy collects almost nothing upfront.

**Concrete requirements:**

After registration, the frontend guides the user through an onboarding flow that collects:

| Field | Type | Required | Validation |
|-------|------|----------|------------|
| `date_of_birth` | date | Yes | Must be 13+ years old, not future date |
| `sex` | enum | Yes | `male`, `female`, `other` |
| `height_cm` | float | Yes | 100–250 cm |
| `weight_kg` | float | Yes | 30–300 kg |
| `activity_level` | enum | Yes | `sedentary`, `lightly_active`, `moderately_active`, `very_active`, `extremely_active` |
| `fitness_goal` | enum | Yes | `lose_fat`, `maintain`, `build_muscle` |
| `unit_preference` | enum | Yes | `metric`, `imperial` |

- `PUT /api/users/me/profile` — update any subset of these fields
- `GET /api/users/me/profile` — return full profile
- The app must calculate and store initial TDEE estimate using Mifflin-St Jeor equation:
  - Male: `10 × weight_kg + 6.25 × height_cm − 5 × age − 161`... wait no: `10 × weight_kg + 6.25 × height_cm − 5 × age + 5`
  - Female: `10 × weight_kg + 6.25 × height_cm − 5 × age − 161`
  - Multiply by activity factor: sedentary=1.2, lightly=1.375, moderate=1.55, very=1.725, extreme=1.9
- Based on goal, derive macro targets:
  - `lose_fat`: TDEE × 0.80, protein = 2.0g/kg, fat = 0.8g/kg, carbs = remainder
  - `maintain`: TDEE × 1.0, protein = 1.8g/kg, fat = 0.9g/kg, carbs = remainder
  - `build_muscle`: TDEE × 1.10, protein = 2.2g/kg, fat = 1.0g/kg, carbs = remainder
- These targets are stored and displayed on the dashboard. User can manually override any target.

### 2.2 Onboarding UX

**Benchmark:** MacroFactor's onboarding is 5 screens, each with one question, progressing left to right with a progress bar. Clean, no clutter, feels fast.

- Onboarding is a multi-step wizard: one field per screen, progress indicator at top
- Each step validates before allowing "Next"
- User can go back to change previous answers
- Final screen shows calculated TDEE and macro targets with an "Adjust" option
- Onboarding must be completable in under 45 seconds
- Skip is not allowed — these fields are needed for nutrition targets

---

## 3. Workout Tracking

### 3.1 Data Model

**What the best apps do:** Hevy tracks exercises with sets, reps, weight, RPE, set type (warmup/working/drop/failure), rest timer, superset grouping, and exercise notes. It shows your previous performance inline so you know what to beat. Strong is nearly identical. Jeff Nippard's Essentials program uses a structured approach with prescribed rep ranges, RIR targets, and progression rules.

**Concrete requirements — Exercise entity:**

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `name` | string | 1–100 chars, unique per user for custom exercises |
| `muscle_group` | enum | `chest`, `back`, `shoulders`, `biceps`, `triceps`, `quadriceps`, `hamstrings`, `glutes`, `calves`, `abs`, `forearms`, `full_body`, `cardio`, `other` |
| `equipment` | enum | `barbell`, `dumbbell`, `machine`, `cable`, `bodyweight`, `kettlebell`, `band`, `other` |
| `is_custom` | bool | false for built-in exercises, true for user-created |
| `created_by` | UUID | null for built-in, user_id for custom |

The app ships with a seed database of **at minimum 200 exercises** covering all major muscle groups and equipment types. This is the Hevy standard — they have 400+. For Phase 1, 200 well-categorized exercises is the floor.

**Concrete requirements — Workout entity:**

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `user_id` | UUID | FK → users |
| `name` | string | 1–100 chars, optional (auto-generated if empty: "Morning Workout", "Afternoon Workout" based on time) |
| `started_at` | timestamp | Required |
| `completed_at` | timestamp | Null while in progress, set on completion |
| `duration_seconds` | int | Computed from started_at/completed_at |
| `notes` | text | Max 2000 chars, optional |
| `status` | enum | `in_progress`, `completed`, `cancelled` |

**Concrete requirements — Exercise Set entity:**

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `workout_id` | UUID | FK → workouts |
| `exercise_id` | UUID | FK → exercises |
| `set_order` | int | 1-indexed within the exercise in this workout |
| `exercise_order` | int | Order of exercise within workout |
| `set_type` | enum | `warmup`, `working`, `drop_set`, `failure` |
| `reps` | int | 0–999. Null if duration-based exercise |
| `weight_kg` | float | 0–999.99. Null for bodyweight |
| `duration_seconds` | int | Null if rep-based. For planks, cardio, etc. |
| `rpe` | float | 1.0–10.0, increments of 0.5. Optional |
| `rest_seconds` | int | Rest taken after this set. Optional |
| `is_pr` | bool | Computed: highest weight×reps for this exercise ever |
| `notes` | string | Max 500 chars, optional |

### 3.2 Workout Logging API

```
POST   /api/workouts                    → Start a new workout (returns workout_id)
GET    /api/workouts/:id                → Get full workout with all sets
PUT    /api/workouts/:id                → Update workout metadata (name, notes)
DELETE /api/workouts/:id                → Delete workout (soft delete, set deleted_at)
POST   /api/workouts/:id/complete       → Mark workout completed, compute duration + PRs

POST   /api/workouts/:id/sets           → Add a set to a workout
PUT    /api/workouts/:id/sets/:set_id   → Update a set
DELETE /api/workouts/:id/sets/:set_id   → Remove a set

GET    /api/workouts?page=1&per_page=20&sort=desc  → Paginated workout history
GET    /api/exercises?search=bench&muscle_group=chest → Search/filter exercises
POST   /api/exercises                   → Create custom exercise
```

### 3.3 Workout Logging UX

**Benchmark — Hevy's logging flow:**
1. User taps "Start Workout" or starts from a saved routine
2. Screen shows exercise list. Tap "Add Exercise" to search/filter the exercise library
3. For each exercise: rows of sets. Each row has columns: Set #, Previous (last session's weight×reps, shown in gray), KG, Reps, checkmark
4. Swiping a set row reveals delete. Long-press to reorder
5. "Add Set" button below the last set for each exercise
6. Rest timer auto-starts when you check off a set. Configurable default (90s, 120s, etc.)
7. Header shows elapsed workout time
8. "Finish Workout" button at top right

**FORGE must match or beat these UX specifics:**

- **Previous performance display**: When logging an exercise, show the last time you did this exercise — weight and reps for each set — in muted text next to the input fields. This is the single most important UX feature in any workout tracker. Without it, the user is guessing.
- **Auto rest timer**: When user completes a set (checks it off), a configurable countdown timer starts (default 90s). Visual + optional audio notification at 0. User can dismiss early.
- **Set type indicators**: Visual badges for Warmup (W), Drop Set (D), Failure (F). Tapping cycles through types.
- **PR detection**: When a set is logged that exceeds the user's all-time best for that exercise (by weight, or by estimated 1RM using Epley formula: `1RM = weight × (1 + reps/30)`), show an animated PR badge immediately. Hevy does this with a trophy icon and confetti-like highlight.
- **Superset grouping**: User can group 2–4 exercises into a superset. Exercises in a superset are visually connected (bracket or color band) and the rest timer only starts after the last exercise in the superset.
- **Exercise search**: Search by name with fuzzy matching. Filter by muscle group and equipment. Results update as user types with debounce (200ms).
- **Workout duration**: Elapsed timer visible in header from moment workout starts. Format: `HH:MM:SS`.
- **Empty state**: First-time user sees "Start your first workout" with a prompt to either start blank or browse exercise templates.

### 3.4 Routine Templates

**What Hevy does:** Users save workout structures as "Routines" (e.g., "Push Day") that can be started with one tap. The routine pre-fills exercises and target sets/reps but doesn't fill in weight — you fill that in during the session based on your previous performance.

- `POST /api/routines` — save a workout structure (exercises + target sets + target rep ranges)
- `GET /api/routines` — list user's saved routines
- `POST /api/routines/:id/start` — start a new workout pre-populated from this routine
- Phase 1 limit: 15 custom routines per user (Hevy free tier allows 3 — be more generous)
- Routine data model: `routine → routine_exercises → routine_sets` (parallel to workout but without actual weights logged)

---

## 4. Nutrition Tracking

### 4.1 Data Model

**What MacroFactor does:** Uses a 24-hour timeline instead of rigid meal slots (breakfast/lunch/dinner). Foods logged to the hour they were consumed. Database is fully verified (unlike MyFitnessPal's unverified user-generated entries). Tracks all macros + 60 micronutrients.

**For Phase 1, FORGE simplifies to:**

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `user_id` | UUID | FK → users |
| `logged_at` | timestamp | When the food was eaten (hour precision in UI) |
| `meal_type` | enum | `breakfast`, `lunch`, `dinner`, `snack` — optional categorization, but food is also shown on a timeline like MacroFactor |
| `food_name` | string | 1–200 chars |
| `brand` | string | Optional, max 100 chars |
| `serving_size` | float | > 0 |
| `serving_unit` | string | `g`, `ml`, `oz`, `cup`, `tbsp`, `tsp`, `piece`, `scoop` |
| `quantity` | float | How many servings. Default 1.0 |
| `calories` | float | Per serving. ≥ 0 |
| `protein_g` | float | Per serving. ≥ 0 |
| `carbs_g` | float | Per serving. ≥ 0 |
| `fat_g` | float | Per serving. ≥ 0 |
| `fiber_g` | float | Optional. ≥ 0 |
| `sugar_g` | float | Optional. ≥ 0 |
| `sodium_mg` | float | Optional. ≥ 0 |
| `is_custom` | bool | True if user-created food entry |
| `source` | enum | `manual`, `database`, `barcode`, `ai` (for Phase 2) |

### 4.2 Food Database

**MacroFactor benchmark:** Verified database of millions of foods. Lab-analyzed nutritional data. Barcode scanner recognizes branded products.

**Phase 1 pragmatic approach:**

- Integrate with the **USDA FoodData Central API** (free, public, lab-verified data) as the primary food database
- `GET /api/foods/search?q=chicken+breast&page=1` — searches USDA database, returns top 20 results with full macro breakdown
- Cache USDA responses in PostgreSQL for 30 days to avoid hitting rate limits and improve latency
- Allow custom food creation: `POST /api/foods/custom` — user enters name + macros manually
- Store user's recent foods: the last 50 unique foods logged, ordered by frequency. This is MacroFactor's "hourly go-tos" concept — the foods you eat repeatedly should be one tap away.
- Search priority: (1) user's recent/frequent foods, (2) user's custom foods, (3) USDA database
- Response time for food search: < 300ms for cached results, < 1000ms for USDA API passthrough

### 4.3 Nutrition Logging API

```
POST   /api/nutrition/log                → Log a food item
GET    /api/nutrition/log?date=2026-02-19  → All foods logged on a date
PUT    /api/nutrition/log/:id             → Edit a logged food (change quantity, macros)
DELETE /api/nutrition/log/:id             → Delete a logged food

GET    /api/nutrition/summary?date=2026-02-19  → Day summary: total cals, protein, carbs, fat vs. targets
GET    /api/nutrition/summary?start=2026-02-10&end=2026-02-19  → Range summary for charts

GET    /api/foods/search?q=...            → Search food database
GET    /api/foods/recent                  → User's recently logged foods (top 50 by frequency)
POST   /api/foods/custom                  → Create custom food entry
```

### 4.4 Nutrition Logging UX

**MacroFactor benchmark:**
- Plate-based logging: add multiple foods to a "plate" then log them all at once
- Timeline view: 24-hour vertical timeline showing when foods were logged
- Fastest food logger on the market by their FLSI metric (fewest taps per food logged)
- "Describe" feature: type or speak a meal description, AI finds matching foods

**FORGE Phase 1 UX requirements:**

- **Food log page**: Shows the current day's foods on a vertical timeline grouped by meal type (breakfast/lunch/dinner/snack), with total calories and macros for each group and the full day
- **Quick add flow**: Tap "+" → search box appears → type food name → results appear (recent foods first, then USDA) → tap food → adjust serving size/quantity → "Log" button. **Must be completable in ≤ 5 taps for a previously-logged food**
- **Macro summary bar**: Persistent bar at top of food log showing: `eaten / target` for calories, protein, carbs, fat. Uses progress bars that fill left-to-right. Color: green when under target, yellow within 10%, red when exceeded
- **Copy meal**: Long-press a meal group → "Copy to today" (for people who eat the same breakfast every day). This is a huge time saver that MacroFactor and MFP both offer
- **Recent foods list**: Below search box, before user types anything, show grid of 8–12 most frequently logged foods with one-tap logging at default serving size
- **Day navigation**: Swipe left/right or tap arrows to view previous/next day's food log
- **Edit flow**: Tap any logged food to edit quantity, serving size, or macros. Changes reflect immediately in daily totals

---

## 5. Dashboard

### 5.1 Dashboard Layout

**MacroFactor benchmark:** Customizable dashboard with widgets — Nutrition & Targets (weekly bar chart), Energy Balance (monthly view), Daily Nutrition, Expenditure trend, Weight Trend, Goal Progress, Habits, Body Metrics. Information-dense but clean.

**Strava benchmark:** Activity feed as the primary view. Today's stats at top, friend activity below.

**FORGE Phase 1 dashboard (no social yet — that's Phase 3):**

The dashboard is the first screen after login. It answers the question: **"How is today going?"**

**Top section — Today's Summary:**
- Circular progress rings (inspired by Apple Watch, not MacroFactor):
  - **Calories**: ring fills as you eat toward target. Shows `eaten / target` numerically
  - **Protein**: separate ring. Most important macro for your user base (lifters)
  - **Carbs**: ring
  - **Fat**: ring
- Below rings: "Log Food" CTA button

**Middle section — Today's Workout:**
- If workout completed today: card showing workout name, duration, total volume (sets × reps × weight summed across all exercises), and number of PRs hit
- If no workout today: "Start Workout" CTA button
- Below: mini calendar row (Mon–Sun) showing which days this week had workouts (filled dots) vs. not (empty dots). Visual streak indicator.

**Bottom section — Weekly Trends:**
- Small bar chart: daily calories for the past 7 days vs. target line
- Small line chart: daily protein intake for the past 7 days vs. target line
- Tap either chart → navigates to full analytics page (Phase 1 keeps this minimal — just larger versions of these charts with 7/14/30 day toggles)

### 5.2 Dashboard API

```
GET /api/dashboard
```

Returns a single aggregated payload:

```json
{
  "today": {
    "date": "2026-02-19",
    "nutrition": {
      "calories": { "consumed": 1450, "target": 2400 },
      "protein_g": { "consumed": 145, "target": 176 },
      "carbs_g": { "consumed": 120, "target": 280 },
      "fat_g": { "consumed": 55, "target": 80 }
    },
    "workout": {
      "completed": true,
      "name": "Push Day",
      "duration_seconds": 3840,
      "total_volume_kg": 12500,
      "exercises_count": 6,
      "sets_count": 22,
      "prs_count": 1
    }
  },
  "week": {
    "workout_days": [true, false, true, true, false, false, false],
    "daily_calories": [2350, 2100, 2400, 1450, null, null, null],
    "daily_protein": [170, 155, 178, 145, null, null, null],
    "calorie_target": 2400,
    "protein_target": 176,
    "current_streak": 3
  }
}
```

- Response time: < 200ms. This is the most-loaded endpoint — cache aggressively. Invalidate cache on any workout or nutrition log write.

---

## 6. Backend Infrastructure

### 6.1 HTTP Server

**Requirements:**

- Framework: Use **Crow** or **Drogon** as the base HTTP library. Crow is simpler (good for learning), Drogon is higher performance (good for production). Pick one and commit.
- Must support: GET, POST, PUT, DELETE, OPTIONS (CORS preflight)
- CORS middleware: Allow requests from `http://localhost:5173` (Vite dev server) and the production frontend domain
- Request logging middleware: log method, path, status code, response time for every request. Format: `[2026-02-19 14:23:01] GET /api/dashboard 200 45ms`
- Auth middleware: extracts JWT from `Authorization: Bearer <token>` header, validates, injects `user_id` into request context
- Error handling: all errors return JSON: `{ "error": { "code": "NOT_FOUND", "message": "Workout not found" } }`
- Request body size limit: 10MB (for future image uploads)
- Graceful shutdown: on SIGTERM, stop accepting new connections, finish in-flight requests (5s timeout), close DB pool, exit

### 6.2 Database

- **PostgreSQL 16+** via **libpqxx**
- Connection pool: min 5, max 20 connections. Connection timeout: 5s. Idle timeout: 300s.
- All tables use UUID primary keys (generated by the application with `uuid_generate_v4()` or C++ UUID library)
- All tables have `created_at` and `updated_at` timestamps, auto-managed
- Soft delete: tables that support deletion have `deleted_at` timestamp. Queries filter `WHERE deleted_at IS NULL` by default
- Migrations: SQL files in `migrations/` directory, numbered sequentially (`001_create_users.sql`, `002_create_workouts.sql`, etc.). A `schema_migrations` table tracks which have been applied. A CLI command `forge migrate` applies pending migrations.
- Indexes required:
  - `users.email` (unique)
  - `users.username` (unique)
  - `workouts.user_id, started_at DESC`
  - `exercise_sets.workout_id`
  - `nutrition_log.user_id, logged_at`
  - `refresh_tokens.token_hash`

### 6.3 Configuration

- All config via environment variables (12-factor app):
  - `DATABASE_URL` — PostgreSQL connection string
  - `JWT_SECRET` — 256-bit signing key
  - `PORT` — HTTP server port (default 8080)
  - `LOG_LEVEL` — debug, info, warn, error
  - `CORS_ORIGINS` — comma-separated allowed origins
  - `USDA_API_KEY` — for food database integration
- No config files checked into git. Provide a `.env.example` with placeholder values.

### 6.4 Build System

- **CMake 3.20+**
- Project compiles with `-Wall -Wextra -Wpedantic -Werror` (treat warnings as errors)
- Debug build: `-g -O0 -fsanitize=address,undefined` (ASan + UBSan always on in dev)
- Release build: `-O2 -DNDEBUG`
- `cmake --build . --target test` runs all unit tests
- `cmake --build . --target bench` runs benchmarks
- Dependencies managed via CMake's `FetchContent` or vcpkg. Document the choice in README.

### 6.5 Testing

- **Google Test** for unit tests
- Coverage target: ≥ 80% line coverage on `services/` and `models/` layers
- Required test suites:
  - Auth: registration validation, login flow, JWT creation/verification, refresh token rotation, rate limiting
  - Workouts: CRUD operations, PR detection logic, pagination, routine creation
  - Nutrition: food logging, daily summary aggregation, search integration, macro calculation
  - Profile: TDEE calculation correctness (verify against known calculators), input validation
- Integration tests: test full HTTP request → response cycle for each endpoint using a test database
- CI: GitHub Actions workflow that runs `cmake build` + `ctest` on every push. Build must pass before merge.

---

## 7. Frontend

### 7.1 Tech Stack

- React 18+ with TypeScript (strict mode)
- Vite as build tool
- Tailwind CSS for styling
- React Router v6 for routing
- TanStack Query (React Query) for server state
- Zustand for client state (rest timer, in-progress workout)
- Recharts for dashboard charts
- Framer Motion for micro-animations (PR confetti, progress ring animations, page transitions)

### 7.2 Route Structure

```
/                       → Dashboard (redirect to /login if not authenticated)
/login                  → Login page
/register               → Registration page
/onboarding             → Profile setup wizard (after first registration)
/workout                → Active workout view (only when workout in progress)
/workout/history        → Paginated workout history
/workout/:id            → Workout detail view (completed workout)
/nutrition              → Today's food log + logging interface
/nutrition/history      → Day-by-day nutrition history
/settings               → User settings (profile, units, rest timer default, logout)
```

### 7.3 Design System

**Benchmarks:**
- MacroFactor: clean, modern, data-dense. Dark mode default. Uses a warm color palette (orange accent). Typography is clear, lots of whitespace.
- Strava: orange accent on dark/white. Bold typography. Activity cards with clear visual hierarchy.
- Hevy: dark blue/gray with green accents for completed sets. Very compact — optimized for in-gym use.

**FORGE design system:**

- **Color palette:**
  - Background: `#0A0A0F` (near-black), surface: `#14141F` (dark card), surface-elevated: `#1E1E2E`
  - Primary accent: `#6C5CE7` (electric purple) — used for CTAs, progress rings, active states
  - Success: `#00D68F` (green) — completed sets, under-target macros
  - Warning: `#FFB800` (amber) — approaching target
  - Danger: `#FF5252` (red) — over target, errors
  - PR/celebration: `#FFD700` (gold) — PR badges, achievement highlights
  - Text primary: `#F0F0F0`, text secondary: `#8888A0`, text muted: `#555570`
- **Typography:**
  - Headings: Inter Bold. Sizes: H1=28px, H2=22px, H3=18px
  - Body: Inter Regular 16px. Line height 1.5
  - Data/numbers: JetBrains Mono or Inter Tabular Nums (monospace-like alignment for columns of numbers)
- **Spacing:** 4px base unit. Padding: 8, 12, 16, 24, 32px
- **Border radius:** 8px for cards, 12px for modals, 24px for buttons, 999px for pills/badges
- **Elevation:** Cards use `box-shadow: 0 2px 8px rgba(0,0,0,0.3)` — subtle depth on dark backgrounds
- **Animations:**
  - Page transitions: 200ms ease-out slide
  - Progress rings: 800ms ease-out fill on dashboard load
  - PR badge: scale bounce (0→1.2→1.0 over 400ms) + gold particle burst
  - Set completion check: 150ms scale pulse
  - Number changes: 200ms counter animation (old value → new value)

### 7.4 Responsive Breakpoints

- Mobile-first design. Primary target: 375px–428px width (iPhone)
- Tablet: 768px+ — two-column layout where appropriate
- Desktop: 1024px+ — max content width 800px, centered. Sidebar navigation.
- The workout logging screen MUST be usable one-handed on a phone. This is gym software. No tiny touch targets. Minimum tap target: 44×44px (Apple HIG standard).

---

## 8. Infrastructure & DevOps

### 8.1 Docker Setup

```yaml
# docker-compose.yml services:
backend:
  build: ./backend
  ports: ["8080:8080"]
  environment: [DATABASE_URL, JWT_SECRET, ...]
  depends_on: [db]

frontend:
  build: ./frontend
  ports: ["5173:5173"]

db:
  image: postgres:16-alpine
  volumes: [pgdata:/var/lib/postgresql/data]
  environment: [POSTGRES_USER, POSTGRES_PASSWORD, POSTGRES_DB]
  ports: ["5432:5432"]
```

- `docker-compose up` should bring up the entire stack from zero
- Backend Dockerfile: multi-stage build. Stage 1: compile with all build tools. Stage 2: copy binary into slim Debian image
- Frontend Dockerfile: build with Node, serve with Nginx
- Database data persisted via Docker volume

### 8.2 Local Development

- `make dev` — starts backend with hot-ish reload (recompile on save using `entr` or similar)
- `make test` — runs all tests
- `make lint` — runs clang-tidy and clang-format checks
- `make migrate` — applies database migrations
- Code formatted with clang-format. Style: LLVM with 100-char line width. Enforced in CI.
- README documents: prerequisites, setup steps, how to run, how to test, how to contribute

---

## 9. Performance Budgets

| Metric | Target | Measurement |
|--------|--------|-------------|
| Dashboard API response | < 200ms p95 | Server-side timer |
| Workout log write | < 100ms p95 | Server-side timer |
| Food search (cached) | < 300ms p95 | Server-side timer |
| Food search (USDA API) | < 1000ms p95 | Server-side timer |
| Frontend initial load | < 2s on 4G | Lighthouse |
| Frontend bundle size | < 500KB gzipped | Build output |
| Memory usage (backend idle) | < 50MB RSS | `ps` monitoring |
| Memory usage (backend under load) | < 200MB RSS | Load test with 100 concurrent users |

---

## 10. Definition of Done

Phase 1 is complete when **all** of the following are true:

- [ ] User can register, log in, and complete onboarding
- [ ] User sees personalized macro targets on dashboard based on their profile
- [ ] User can start a workout, add exercises from library, log sets with weight/reps/RPE, and complete the workout
- [ ] Previous workout performance displays inline during logging
- [ ] Rest timer auto-starts on set completion
- [ ] PR detection works and shows visual celebration
- [ ] User can save and start workouts from routine templates
- [ ] User can search the USDA food database and log foods with full macros
- [ ] User can create custom foods
- [ ] Recent/frequent foods appear first in search
- [ ] Dashboard shows today's nutrition progress, today's workout summary, and 7-day trends
- [ ] All API endpoints have integration tests
- [ ] `docker-compose up` brings up the full working stack
- [ ] CI pipeline passes: build + tests + lint
- [ ] The app is usable on a phone screen (375px wide, one-handed operation for workout logging)
- [ ] Backend runs with ASan in debug mode with zero memory errors

**If you can open FORGE on your phone at the gym, log your Garou Protocol session with previous weights showing, log your post-workout meal by searching "chicken breast", and see your daily progress on the dashboard — Phase 1 is done.**