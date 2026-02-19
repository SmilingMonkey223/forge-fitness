# FORGE — Complete Project Specification

> **This is the single source of truth for the entire FORGE project. Give this document to Claude Code. It contains everything: project vision, competitive landscape, tech stack, and detailed functional specifications for all four phases.**

---

# Table of Contents

1. [Project Overview & Vision](#1-project-overview--vision)
2. [Competitive Landscape](#2-competitive-landscape)
3. [Tech Stack & Architecture](#3-tech-stack--architecture)
4. [Phase 1: Foundation](#4-phase-1-foundation)
5. [Phase 2: AI & Intelligence](#5-phase-2-ai--intelligence)
6. [Phase 3: Community & Social](#6-phase-3-community--social)
7. [Phase 4: Polish & Scale](#7-phase-4-polish--scale)
8. [Project Structure](#8-project-structure)
9. [Design System](#9-design-system)
10. [Success Metrics](#10-success-metrics)

---

# 1. Project Overview & Vision

## What is FORGE?

FORGE is a free, open-source fitness platform that combines workout tracking, AI-powered nutrition management, and social community features into one cohesive app. It's built with a C++ backend and React/TypeScript frontend.

**The gap it fills:** No app in the market today does all three of these things well — nutrition tracking at MacroFactor quality + workout logging at Hevy quality + social features at Strava quality. FORGE is the first free app that does all three in one product.

## Why Build This?

1. **Learn C++ deeply** — the backend touches real systems programming: HTTP servers, PostgreSQL, ONNX ML inference, image processing, WebSocket messaging, thread pools, async processing, memory management under load
2. **Build something actually useful** — people pay real money for apps that do subsets of what FORGE does
3. **Portfolio-defining project** — open source, ambitious, demonstrates every hard part of C++
4. **Community value** — free alternative to paid fitness apps, with community-driven workout content

## Core Principles

- **Free forever, no ads** — community is the moat, not paywalled features
- **Fast in the gym** — workout logging must be one-handed, minimal taps, instant feedback
- **Smart about nutrition** — AI-assisted food logging, adaptive TDEE that learns your real metabolism
- **Fun because of community** — social feed, workout sharing, friendly competition
- **Evidence-based** — algorithms and recommendations backed by sports science research
- **Adherence-neutral** — never shame users for missing targets (MacroFactor philosophy)

## Phase Roadmap

| Phase | Focus | Duration | Key Deliverables |
|-------|-------|----------|-----------------|
| **Phase 1: Foundation** | Auth, workout logging, nutrition logging, dashboard | 4–6 weeks | Working app for daily gym use and food tracking |
| **Phase 2: Intelligence** | AI food recognition, weight trending, adaptive TDEE, analytics, progress photos, gamification | 4–6 weeks | AI photo → calories. Trend line. Exercise charts. Weekly coaching. |
| **Phase 3: Community** | Social feed, friends, messaging, workout marketplace, clubs | 6–8 weeks | Follow friends, see activity, browse/upvote community workouts, chat |
| **Phase 4: Polish** | Offline support, PWA, performance optimization, data export, onboarding refinement | 4–6 weeks | Production-ready. Performant under load. Installable on mobile. |

---

# 2. Competitive Landscape & Strategic Foundation

> **This document replaces the original project prompt. It maps every top product in the fitness app market, identifies what they do best, where they fail, and defines exactly how FORGE should position itself relative to them. All phase specs should reference this document for design decisions.**

---

## 1. Market Overview

The fitness app market was ~$12–14 billion in 2025, growing at 12–13% CAGR. The top 10 apps control 60%+ of downloads. The market is fragmented by function — no single app does everything well:

| Category | Market Leaders | Monthly Revenue (Top App) | Gap FORGE fills |
|----------|---------------|--------------------------|-----------------|
| Nutrition tracking | MyFitnessPal ($12M/mo), MacroFactor, Lose It! | $12M | MFP has unverified data and bloated UX. MacroFactor is excellent but has no workout tracking and no social |
| Workout logging | Hevy (~5M users), Strong, JEFIT (8M+) | ~$500K–1M | None combine nutrition + workout tracking + social in one app with this quality |
| Social fitness | Strava ($5.7M/mo) | $5.7M | Strava is running/cycling only. No gym/lifting community exists at this scale |
| AI coaching | Fitbod (15M downloads), SensAI, Dr. Muscle | ~$1M | AI coaching exists but is isolated — no app combines AI food recognition + AI workout coaching + community |
| All-in-one | MyFitnessPal (attempts it), FitBod (attempts it) | — | Nobody does all three (nutrition + training + social) at a high quality level. This is the gap. |

**FORGE's strategic position: The first free, high-quality app that does all three pillars (nutrition tracking at MacroFactor level + workout logging at Hevy level + social at Strava level) in one cohesive product.**

---

## 2. Detailed Competitor Breakdown

### 2.1 MyFitnessPal — The Revenue King ($12M/mo, 5M+ MAU)

**What they do well:**
- Largest food database (14M+ foods) — nearly everything scannable
- Brand recognition — "calorie tracking" = MFP in most people's minds
- Barcode scanner works globally
- Basic workout logging exists
- Integration with hundreds of wearables and apps

**What they do poorly:**
- Food database is unverified user-generated content — full of duplicates and incorrect entries. MacroFactor's verified DB is far more accurate.
- UX is bloated and dated. Logging food takes more taps than any major competitor (1.5× more than MacroFactor per their FLSI study).
- Premium is expensive ($20/mo or $80/yr) and locks basic features like macro targets behind paywall
- No adaptive TDEE. Uses static calorie goals that never adjust to your actual metabolism.
- Workout tracking is an afterthought — bare minimum functionality
- No social feed or community features worth mentioning
- Aggressively ad-supported on free tier

**What FORGE takes from MFP:** Large food database access (via USDA API + user-submitted verified entries), barcode scanning concept. **What FORGE does better:** Verified food data only, faster logging UX, adaptive TDEE, workout tracking that's actually good, no ads ever.

---

### 2.2 MacroFactor — The Science King ($6/mo, ~250K+ users)

**What they do well:**
- Best-in-class food logging speed (quantifiably fastest per their FLSI benchmark — fewest taps across 20 apps tested)
- Verified food database — lab-analyzed entries, no user-submitted garbage
- Adaptive expenditure algorithm (V3) that calculates your real TDEE from your logged food + weight changes. This is genuinely innovative — no competitor has anything close.
- Weight trend smoothing via EWMA — filters daily noise, shows real trajectory
- Weekly coaching check-ins that adjust calorie/macro targets based on actual progress
- Adherence-neutral philosophy — doesn't shame you for missing targets
- Micronutrient tracking (60 nutrients)
- AI food logging (photo + natural language description → food identification)
- Customizable dashboard with drag-and-drop widgets
- Excellent UX design — clean, modern, data-dense without being cluttered
- Founded by Greg Nuckols (Stronger By Science) — deep credibility in evidence-based fitness

**What they do poorly:**
- No workout tracking (they're building "MacroFactor Workouts" but it's still in beta as of late 2025)
- No social features at all — completely solo experience
- No community workout sharing or discovery
- Paid only ($6/mo or $72/yr) — no free tier
- No exercise form guidance

**What FORGE takes from MacroFactor:** Adaptive TDEE concept, weight trend smoothing, fast food logging design patterns, adherence-neutral tone, verified food data approach, AI food recognition. **What FORGE does better:** Combines nutrition with full workout tracking and social community in one app. Free tier.

---

### 2.3 Strava — The Social King ($5.7M/mo, 180M+ users)

**What they do well:**
- The social feedback loop is the product: exercise → post → kudos/comments → motivation → more exercise. Incredibly effective for retention.
- 1M+ clubs. Social infrastructure that drives real-world community (run clubs, cycling groups)
- Activity feed that makes other people's workouts visible and motivating
- Segments and leaderboards create friendly competition
- "Kudos" system — simple, non-verbal encouragement that has very low friction
- Real-time activity tracking with GPS
- Year-in-Sport recap — brilliant retention/engagement feature
- Strava's brand is aspirational — being on Strava signals you're "an athlete"
- Club events integrate into the feed, driving IRL meetups

**What they do poorly:**
- Almost entirely cardio-focused (running, cycling, swimming). Gym/lifting is an afterthought.
- No set/rep/weight tracking. If you lift, Strava is useless for tracking your actual workout.
- No nutrition tracking at all
- Recent UI redesigns have been poorly received (2025 Android update widely criticized)
- Freemium model locks most analytics behind $12/mo subscription
- No AI features for workout guidance

**What FORGE takes from Strava:** Activity feed design, kudos/reaction concept, club/community infrastructure, social motivation loop, year-in-review concept. **What FORGE does better:** Actually useful for gym/lifting athletes, integrated nutrition tracking, workout plan sharing marketplace.

---

### 2.4 Hevy — The Workout Logger King (~10M users, free tier dominant)

**What they do well:**
- Best free-tier workout tracking UX. Clean, intuitive, fast logging.
- 400+ exercises with video demos and muscle group targeting
- Previous performance inline display — THE killer feature. Shows last session's weight/reps right next to your input fields
- Auto rest timer with configurable defaults
- Set types: warmup, working, drop set, failure
- Superset grouping
- PR detection with celebration UI
- Social feed (Strava-like) — can follow friends, see their workouts, give kudos
- Routine templates — save and reuse workout structures
- Apple Watch + Wear OS support
- Progress charts: exercise progression, muscle distribution, volume over time
- Coach platform (Hevy Coach) for trainers to program and monitor clients
- Community workout sharing — browse and copy other users' routines
- Generous free tier (unlimited workouts, unlimited history)

**What they do poorly:**
- No nutrition tracking at all
- No adaptive training guidance — purely a logger, not a coach
- Analytics are basic compared to what's possible (no estimated 1RM progression, limited volume analysis)
- Social features exist but are surface-level compared to Strava
- No AI features (no auto-progression, no recovery-based recommendations)

**What FORGE takes from Hevy:** Workout logging UX (inline previous performance, rest timer, set types, superset groups, PR detection), social feed for workouts, routine templates, exercise library with muscle group categorization. **What FORGE does better:** Adds nutrition tracking, AI food recognition, adaptive TDEE, deeper analytics, workout marketplace with upvotes/trending.

---

### 2.5 Fitbod — The AI Workout King (15M downloads, $13/mo)

**What they do well:**
- AI generates personalized workouts based on equipment, goals, experience level, and recovery status
- Tracks which muscle groups are fatigued and programs around recovery
- Automatically handles progressive overload — increases weight/reps based on performance
- Visual muscle map showing recovery status (fresh → fatigued)
- Clean exercise demonstrations with form guidance
- Equipment-flexible — adapts to whatever you have available
- Integrates with Apple Health for activity data

**What they do poorly:**
- No free tier — 3 free workouts, then $13/mo paywall
- AI can feel rigid — advanced lifters who follow specific programs (PPL, 5/3/1, etc.) find it interferes with their planning
- No nutrition tracking
- No social features
- Not designed for experienced lifters with established routines
- No community or workout sharing

**What FORGE takes from Fitbod:** The concept of muscle group recovery tracking, AI-assisted progressive overload (future phase), visual muscle map. **What FORGE does better:** Lets experienced lifters run their own programs while optionally using AI suggestions. Free. Social. Nutrition integrated.

---

### 2.6 Strong — The Minimalist Logger

**What they do well:**
- Clean, no-nonsense interface. Zero distractions.
- Fast workout logging
- Plate calculator
- 1RM estimation
- Data export to CSV

**What they do poorly:**
- Free tier limited to 3 custom exercises — aggressively pushes to paid ($30/yr)
- No social features
- No nutrition tracking
- Limited analytics
- Slower development compared to Hevy
- Exercise library smaller than competitors

**What FORGE takes from Strong:** Minimalism as a design principle for the active workout screen — when logging a set, nothing should distract. **What FORGE does better:** Everything else.

---

### 2.7 JEFIT — The Library King (8M+ users)

**What they do well:**
- 1,400+ exercises — largest library of any workout app
- Each exercise has video demonstrations, muscle group targeting, and detailed instructions
- Community-created workout programs browsable by goal
- Social features with workout sharing

**What they do poorly:**
- Dated UI — feels 5 years behind Hevy/Strong
- Ads in free version are intrusive
- Performance issues reported (slow loading, occasional crashes)
- No nutrition tracking
- Analytics are basic

**What FORGE takes from JEFIT:** Large exercise library concept, community-created programs browsable by category. **What FORGE does better:** Modern UI, better performance (C++ backend), integrated nutrition, no ads.

---

### 2.8 Emerging Competitors Worth Watching

| App | What's interesting | Threat level |
|-----|-------------------|--------------|
| **Alpha Progression** | German-made, hypertrophy-focused with auto-progression logic. Gaining fans for structured periodization. | Medium — niche but growing |
| **Setgraph** | Speed-optimized logging. Claims fastest logging flow. Flow-state focused design. | Low — small user base |
| **SensAI** | Uses wearable data (HRV, sleep) to adjust workout intensity in real-time. Most sophisticated recovery-based programming. | Medium — innovative but unproven at scale |
| **SnapCalorie** | LIDAR-based food volume estimation. 16% mean calorie error — best accuracy in the market. Now free. | Medium — great tech, narrow focus |
| **Cal AI** | LLM-powered food logging via photos. Influencer-marketed, fast-growing. | Medium — hype-driven but the UX is good |
| **MacroFactor Workouts** | MacroFactor is building workout tracking. If they ship well, they become the most direct competitor to FORGE's vision. | **High** — same all-in-one vision, backed by Greg Nuckols' credibility |

---

## 3. The Gap FORGE Fills

No app in the market today does all three of these things well:

```
┌─────────────────────┐
│   NUTRITION          │  ← MacroFactor, MyFitnessPal
│   (food logging,     │
│    AI recognition,   │
│    adaptive TDEE)    │
├─────────────────────┤
│   TRAINING           │  ← Hevy, Strong, Fitbod
│   (workout logging,  │
│    analytics,        │
│    progression)      │
├─────────────────────┤
│   COMMUNITY          │  ← Strava (cardio only)
│   (social feed,      │
│    workout sharing,  │
│    marketplace)      │
└─────────────────────┘

FORGE = all three, cohesive, free, for lifters
```

The closest competitor to FORGE's full vision is **MacroFactor** (if/when they ship their Workouts feature and add social). That is the competitive threat to track most closely.

---

## 4. Feature Priority Matrix (What to Build, Benchmarked)

Every feature FORGE builds should match or exceed the best-in-class for that feature:

| Feature | Best-in-class benchmark | FORGE target | Phase |
|---------|------------------------|--------------|-------|
| Food logging speed | MacroFactor (fewest taps in FLSI) | ≤ 5 taps for recent food | 1 |
| Food database quality | MacroFactor (verified) | USDA verified + user submissions with review | 1 |
| Workout logging UX | Hevy (inline previous, rest timer, set types) | Match Hevy exactly, then add nutrition context | 1 |
| Exercise library size | JEFIT (1,400+), Hevy (400+) | 200+ at launch, community-submitted growth | 1 |
| Previous performance display | Hevy/Strong (inline gray text) | Match exactly | 1 |
| PR detection | Hevy (trophy icon, instant detection) | Match + add 1RM PR type | 1 |
| Adaptive TDEE | MacroFactor (V3 algorithm) | Simplified EWMA-based approach | 2 |
| Weight trend smoothing | MacroFactor (proprietary EWMA variant) | Standard EWMA, α=0.1 | 2 |
| AI food recognition | SnapCalorie (16% error), Cal AI, MacroFactor AI | Tier 1 ONNX + Tier 2 LLM hybrid | 2 |
| Exercise progression charts | Hevy (weight over time, volume) | Match + add estimated 1RM chart | 2 |
| Muscle group distribution | Hevy (pie chart) | Match + add weekly volume bars with MEV/MRV lines | 2 |
| Progress photos | MacroFactor (3 angles, body metrics) | Match + add comparison slider | 2 |
| Social activity feed | Strava (activity cards, kudos, comments) | Adapted for gym: workout cards with volume/PRs, kudos | 3 |
| Friend/follow system | Strava (follow model) | Match | 3 |
| Workout sharing/marketplace | Hevy (share routines), JEFIT (community programs) | Exceed: upvotes, trending, sport/goal filters | 3 |
| Messaging | None (no fitness app does this well) | 1-on-1 chat + workout discussion threads | 3 |
| Clubs/groups | Strava (1M+ clubs) | Gym-focused clubs with shared workout challenges | 3 |
| Gamification | Strava (kudos, segments), Hevy (PR celebration) | Badges, streaks, streak freezes, year-in-review | 2–3 |
| Offline support | Strong, FitNotes, Hevy (all work offline) | Full offline workout logging, sync on reconnect | 4 |
| Wearable integration | Hevy (Apple Watch, Wear OS), MacroFactor (Apple Watch) | Apple Health sync (future: Watch app) | 4 |
| Data export | Strong (CSV), Hevy (CSV), MacroFactor (CSV) | Full CSV/JSON export of all data | 4 |

---

## 5. Design Philosophy (Synthesized from Best-in-Class)

### From MacroFactor: **Data density without clutter**
- Show lots of information on screen but with clear visual hierarchy
- Use whitespace strategically — not minimalism for its own sake, but minimalism that makes the data readable
- Adherence-neutral tone: never shame the user. "You ate 2,800 kcal today" not "You went 400 over your goal! 🚫"

### From Hevy: **Speed during active logging**
- When you're in the gym holding a phone between sets, every tap matters
- Previous performance visible without navigating anywhere
- Rest timer starts automatically
- One-handed operation is non-negotiable

### From Strava: **The social loop is the retention engine**
- People don't come back to Strava for the analytics. They come back because their friends are there.
- Make posting effortless (auto-generated workout summaries)
- Make engagement low-friction (kudos = one tap, no text required)
- Make discovery natural (trending workouts, friend activity)

### From Fitbod: **Intelligence that augments, not replaces**
- AI should suggest, not dictate. Experienced lifters want control.
- Show the user why a recommendation was made (e.g., "Your chest volume is below 10 sets/week — consider adding a set")
- Recovery tracking is valuable context, not a hard constraint

### FORGE's synthesis:
> **Fast in the gym. Smart about your nutrition. Fun because of the community. Free because it should be.**

---

## 6. Updated Phase Structure

| Phase | Focus | Duration | Key deliverables |
|-------|-------|----------|------------------|
| **Phase 1: Foundation** | Auth, workout logging (Hevy-quality), nutrition logging (MacroFactor-speed), dashboard | 4–6 weeks | Usable at the gym for daily workouts and food logging |
| **Phase 2: Intelligence** | AI food recognition, weight trending, adaptive TDEE, deep analytics, progress photos, gamification | 4–6 weeks | AI photo → calories. Weight trend line. Exercise progression charts. Weekly coaching check-ins. |
| **Phase 3: Community** | Social feed, friends, messaging, workout marketplace, clubs, reactions | 6–8 weeks | Follow friends, see their activity, browse/upvote community workouts, chat |
| **Phase 4: Polish** | Offline support, PWA, onboarding refinement, rate limiting, caching, performance optimization, data export | 4–6 weeks | Production-ready. Performant under load. Installable on mobile. |

---

## 7. Monetization Strategy (Free-First)

FORGE is free. Period. No ads. No paywalled basic features.

**Why free works here:** The community is the product. More users = more community workouts = more social engagement = more retention. Paywalling kills network effects.

**Future revenue options (Phase 5+, not in scope now):**
- **Premium analytics**: advanced periodization charts, AI-generated training suggestions, recovery scoring → $5/mo
- **Coach platform**: like Hevy Coach — trainers can program and monitor clients through FORGE → $20/mo per trainer
- **Verified creator badges**: for certified trainers who publish workout programs
- **API access**: for developers building fitness tools on top of FORGE data

But none of this matters until the product is great and the community exists. Build first.

---

## 8. Success Metrics

| Metric | Target (6 months post-launch) | Why this matters |
|--------|-------------------------------|------------------|
| Workout logging sessions/week per active user | ≥ 3 | Proves the logger is good enough for daily use |
| Food logs/day per active user | ≥ 2 meals logged | Proves nutrition tracking is fast enough to stick |
| 30-day retention | ≥ 40% | Industry average is ~25%. Strava is ~60%. Social features drive retention. |
| Community workouts published | ≥ 500 | Critical mass for the marketplace to feel alive |
| Average session duration | 3–8 minutes | Too short = not engaging. Too long = UX friction. |

---

## 9. Open Source Strategy

FORGE will be open-source (MIT license). This is a strategic decision:

1. **Trust**: Users can verify their data isn't being misused
2. **Contributions**: Community can submit exercises, fix bugs, improve translations
3. **Portfolio value**: An open-source fitness platform with thousands of stars is orders of magnitude more impressive than a private repo
4. **Moat**: The moat isn't the code — it's the community and data network effects. Open-sourcing the code doesn't give away the moat.

The underlying multi-agent protocol work from FORGE (the forge-protocol library) could be extracted as a separate open-source library for the broader AI community.

---

**This document is the strategic foundation. The Phase 1, 2, 3, and 4 specs provide tactical implementation details. When making any design decision, ask: "What does the best app in this category do? How does FORGE match or beat it?"**
-e 
---

# 3. Tech Stack & Architecture

## Backend (C++)

| Component | Technology | Why |
|-----------|-----------|-----|
| HTTP Framework | Crow or Drogon | Crow = simpler (good for learning), Drogon = higher perf. Pick one, commit. |
| Database | PostgreSQL 16+ via libpqxx | Robust, great JSON support, full-text search with pg_trgm |
| ML Inference | ONNX Runtime 1.17+ | Cross-platform, C++ native, fast CPU inference |
| Image Processing | OpenCV 4.8+ | Industry standard for preprocessing, well-documented C++ API |
| HTTP Client | libcurl with SSL | For LLM API calls, USDA API, external integrations |
| Object Storage | MinIO (S3-compatible) | Progress photos, media uploads. Self-hosted, free |
| Real-time | WebSocket (via Crow/Drogon) | Activity feed updates, messaging, live workout sync |
| JSON | nlohmann/json or RapidJSON | Serialization/deserialization for all API responses |
| Auth | jwt-cpp + OpenSSL + libbcrypt | JWT signing, bcrypt hashing, token management |
| Build | CMake 3.20+ | Standard C++ build system. FetchContent for deps |
| Testing | Google Test | Unit + integration tests. ≥80% coverage target |

## Frontend (React/TypeScript)

| Component | Technology | Why |
|-----------|-----------|-----|
| Framework | React 18+ with TypeScript (strict) | Known stack, rich ecosystem |
| Build | Vite | Fast HMR, instant dev server |
| Styling | Tailwind CSS | Utility-first, responsive, dark mode |
| Routing | React Router v6 | Standard routing |
| Server State | TanStack Query (React Query) | Cache, refetch, optimistic updates |
| Client State | Zustand | Lightweight — rest timer, in-progress workout, auth |
| Charts | Recharts | Dashboard analytics, progression charts |
| Animations | Framer Motion | PR confetti, progress rings, page transitions |
| Real-time | Native WebSocket API | Feed updates, messaging |

## Infrastructure

| Component | Technology |
|-----------|-----------|
| Containers | Docker + docker-compose |
| Database | PostgreSQL 16 Alpine |
| Object Storage | MinIO |
| CI/CD | GitHub Actions |
| Code Quality | clang-format + clang-tidy |
| Memory Safety | ASan + UBSan in all debug builds |

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    React Frontend                        │
│  (TypeScript, Tailwind, Recharts, Framer Motion)        │
└──────────────┬──────────────────────┬───────────────────┘
               │ REST API             │ WebSocket
               ▼                      ▼
┌─────────────────────────────────────────────────────────┐
│                   C++ Backend (Crow/Drogon)              │
│                                                          │
│  ┌──────────┐ ┌──────────┐ ┌───────────┐ ┌──────────┐  │
│  │   Auth   │ │ Workout  │ │ Nutrition │ │  Social  │  │
│  │ Service  │ │ Service  │ │  Service  │ │ Service  │  │
│  └────┬─────┘ └────┬─────┘ └─────┬─────┘ └────┬─────┘  │
│       │             │             │             │        │
│  ┌────┴─────────────┴─────────────┴─────────────┴────┐  │
│  │              PostgreSQL (libpqxx)                  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ ONNX Runtime │  │   libcurl    │  │    MinIO     │  │
│  │ (ML Inference)│  │ (HTTP Client)│  │(Object Store)│  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │        Thread Pool (4-8 threads)                  │   │
│  │  ML inference, image processing, async tasks      │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

-e # 4. Phase 1: Foundation

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
-e 
---

-e # 5. Phase 2: AI & Intelligence

## Detailed Functional Specification

> **Benchmark apps:** MacroFactor (expenditure algorithm, weight trending, analytics), SnapCalorie (photo-based food recognition with volume estimation), Cal AI (LLM-powered food logging), Hevy (workout analytics, PR tracking, muscle group charts)
>
> **Phase 2 goal:** Point your camera at food and get calorie + macro estimates. Track body weight with intelligent trend smoothing. See deep analytics on both training and nutrition progress. Introduce an adaptive TDEE system that adjusts targets based on real data, not just a static formula.

---

## 1. AI Food Recognition

### 1.1 Architecture Decision: Hybrid Pipeline

**State of the art (2025):**
- Pure on-device CV models (SnapCalorie, Calorie Mama): 60–82% accuracy on real meals. Best with single, separated foods. Struggle with mixed dishes, sauces, homemade meals.
- LLM-based (Cal AI, MacroFactor AI): Send photo to vision model (GPT-4V, Claude), get food identification + estimated macros. More flexible, handles complex meals, but dependent on external API.
- Best systems (SnapCalorie): Use LIDAR/depth sensors for volume estimation → 16% mean calorie error. FORGE won't have LIDAR on a web app, so this isn't viable.
- Research consensus: AI achieves ~80% accuracy on simple foods, ~62% on mixed meals. Manual entry still beats AI at 95%+ accuracy.

**FORGE approach — two-tier hybrid:**

| Tier | Method | When to use | C++ learning value |
|------|--------|-------------|-------------------|
| **Tier 1: On-server classification** | ONNX Runtime running EfficientNet-Lite or MobileNetV3 fine-tuned on Food-101/Food-2K dataset | First pass — fast classification of food category | ONNX Runtime C++ API, image preprocessing, tensor memory management, async inference |
| **Tier 2: LLM refinement** | External API call (Claude or GPT-4V) with the photo + Tier 1 classification results | When Tier 1 confidence < 0.7, or for portion size estimation and multi-item meals | HTTP client (libcurl), JSON parsing, prompt engineering, response parsing |

This hybrid gives you the best of both worlds: real C++ ML inference work *and* high-quality results for complex meals.

### 1.2 Image Upload & Preprocessing Pipeline

**Concrete requirements — Upload endpoint:**

```
POST /api/nutrition/recognize
Content-Type: multipart/form-data
Body: image file (JPEG/PNG/WebP)
```

**Preprocessing pipeline (all in C++):**

| Step | Operation | Implementation | Constraints |
|------|-----------|---------------|-------------|
| 1 | Receive upload | Multipart parser in HTTP handler | Max file size: 10MB. Reject non-image MIME types |
| 2 | Decode image | OpenCV `cv::imdecode()` or stb_image | Support JPEG, PNG, WebP. Reject corrupt/invalid images |
| 3 | EXIF orientation | Read EXIF, apply rotation | Photos from phones often have rotation metadata |
| 4 | Resize | Bilinear interpolation to model input size | 224×224 for MobileNetV3, 300×300 for EfficientNet-Lite |
| 5 | Color space | BGR → RGB (OpenCV loads as BGR) | Must match model training format |
| 6 | Normalize | Pixel values: `/255.0`, then `(x - mean) / std` | ImageNet defaults: mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225] |
| 7 | Tensor layout | HWC → CHW (height×width×channels → channels×height×width) | ONNX models expect NCHW: batch×channels×height×width |
| 8 | Batch dim | Add batch dimension: [3,224,224] → [1,3,224,224] | Single-image inference per request |

**Memory management requirements:**
- All image buffers allocated on the stack or via RAII (`std::vector<float>`)
- No raw `new`/`delete`. Zero memory leaks under load.
- Image data freed immediately after tensor creation — don't hold two copies simultaneously
- Total memory per request: < 50MB (original image + resized + tensor)

**Performance budget:**
- Preprocessing (steps 1–8): < 50ms p95
- Full pipeline (upload → preprocessing → inference → response): < 500ms p95 for Tier 1 only
- Full pipeline with Tier 2 LLM fallback: < 3000ms p95 (external API latency dominated)

### 1.3 ONNX Runtime Integration

**Model selection:**

Use a pre-trained model fine-tuned on food classification. Options ranked by suitability:

| Model | Size | Latency (CPU) | Top-1 Accuracy (Food-101) | Recommendation |
|-------|------|---------------|---------------------------|----------------|
| MobileNetV3-Small | ~6MB | ~5ms | ~82% | Good for starting — fast, small |
| EfficientNet-Lite0 | ~17MB | ~15ms | ~87% | Better accuracy, still fast |
| EfficientNet-B0 | ~21MB | ~25ms | ~89% | Best accuracy for the size |

**Start with MobileNetV3-Small** for initial development (fastest iteration), then upgrade to EfficientNet once the pipeline is stable.

**Where to get the model:**
1. Download a PyTorch model pre-trained on Food-101 (101 food categories) or Food-2K (2000 categories) from Hugging Face or TorchVision
2. Export to ONNX using `torch.onnx.export()`
3. Optimize with ONNX Runtime's graph optimization (level 99 — all optimizations)
4. Optionally quantize to INT8 for 2–3× speedup with minimal accuracy loss

**ONNX Runtime C++ implementation requirements:**

- `Ort::Env` created once at server startup, shared across all requests (thread-safe)
- `Ort::Session` created once at startup with the loaded model
- `Ort::SessionOptions`: set `SetIntraOpNumThreads(4)`, enable `ORT_ENABLE_ALL` graph optimizations
- Per-request: create `Ort::Value` input tensor from preprocessed image data, run inference, read output tensor
- Output: softmax probability vector of size N (where N = number of food classes)
- Extract top-5 predictions with confidence scores
- **Thread safety**: ONNX Runtime sessions are thread-safe for concurrent `Run()` calls. Do NOT create a new session per request.

**Inference result structure:**

```json
{
  "predictions": [
    { "food_class": "grilled_chicken_breast", "confidence": 0.83 },
    { "food_class": "fried_chicken", "confidence": 0.09 },
    { "food_class": "chicken_wings", "confidence": 0.04 },
    { "food_class": "turkey_breast", "confidence": 0.02 },
    { "food_class": "chicken_nuggets", "confidence": 0.01 }
  ],
  "tier": 1,
  "inference_time_ms": 18
}
```

### 1.4 LLM Refinement (Tier 2)

**When to trigger:**
- Tier 1 top prediction confidence < 0.70
- User explicitly requests "more detail" or the meal is visually complex
- Multi-item detection: if Tier 1 returns a generic class like "mixed_plate" or "salad"

**Implementation:**

- `POST` request to Anthropic or OpenAI API with:
  - The original image (base64 encoded)
  - Tier 1 classification results as context
  - A structured prompt requesting: food items identified, estimated portion sizes, calories and macros per item
- Parse JSON response from LLM
- Map identified foods to USDA database entries where possible (for verified nutritional data)
- Fall back to LLM-estimated macros only when no USDA match exists

**Prompt template:**

```
You are a nutrition analysis assistant. Analyze this food photo.

The on-device classifier identified this as: {tier1_top_prediction} ({tier1_confidence}%)

Please identify:
1. Each distinct food item visible
2. Estimated portion size for each (in grams)
3. Estimated calories, protein (g), carbs (g), fat (g) for each item

Respond in JSON format:
{
  "items": [
    {
      "name": "grilled chicken breast",
      "portion_grams": 150,
      "calories": 248,
      "protein_g": 46,
      "carbs_g": 0,
      "fat_g": 5.4
    }
  ],
  "total_calories": 248,
  "total_protein_g": 46,
  "total_carbs_g": 0,
  "total_fat_g": 5.4,
  "confidence_note": "Portion estimated from plate reference"
}
```

**C++ implementation:**
- HTTP POST using libcurl with SSL
- JSON serialization/deserialization with nlohmann/json or RapidJSON
- Timeout: 10 seconds. Retry once on 5xx errors. Return Tier 1 results only if Tier 2 fails.
- Rate limit: max 100 LLM requests per user per day (cost control)

### 1.5 Food Recognition → Nutrition Log Flow

**API endpoint:**

```
POST /api/nutrition/recognize
→ Returns: list of recognized food items with estimated macros

POST /api/nutrition/recognize/confirm
Body: { items: [...adjusted items...] }
→ Logs confirmed items to the user's food log
```

**Frontend UX flow:**

1. User taps camera icon on nutrition page
2. Browser camera opens (MediaDevices API). User takes photo or selects from gallery
3. Image uploads. Show loading spinner with "Analyzing your meal..."
4. Results appear as editable cards — one per detected food item:
   - Food name (editable text field, pre-filled with AI result)
   - Portion size (editable, with g/oz toggle)
   - Calories, protein, carbs, fat (editable, auto-recalculate if portion changes)
   - Confidence indicator: green (>80%), yellow (60–80%), red (<60%)
   - "Search database" link to switch to manual USDA search for this item
5. User reviews, adjusts as needed, taps "Log Meal"
6. All items saved to nutrition log with `source: "ai"` and `confidence_score` stored

**Critical UX principle (from MacroFactor's philosophy):**
> AI results are a *starting point*, not a final answer. The UI must make it trivially easy to edit, replace, or reject any AI suggestion. Never auto-log without user confirmation.

### 1.6 Food Recognition Testing Requirements

- Unit tests for the preprocessing pipeline: verify correct resize, normalization, tensor shape for 10+ test images
- Integration test: upload a known food image (e.g., apple) → verify Tier 1 returns correct class in top-3
- Load test: 10 concurrent recognition requests complete within 2 seconds each
- Edge cases to handle: rotated images, very dark photos, close-up (no plate reference), empty plate, non-food images (return "no food detected" gracefully)
- Memory leak test: run 1000 sequential requests under ASan — zero leaks

---

## 2. Body Weight Tracking & Trend Analysis

### 2.1 What MacroFactor Does (The Gold Standard)

MacroFactor's weight trend system is the most sophisticated in any consumer app. Key properties:
- Uses a weighted moving average that emphasizes recent weigh-ins
- Fills gaps via linear interpolation (missed days don't break the algorithm)
- Separates "signal" (real weight change) from "noise" (water retention, food volume, sodium, etc.)
- Weight trend drives the expenditure calculation, which in turn drives calorie target adjustments
- Resilient to missed days, anomalous weigh-ins, and short-term fluctuations

### 2.2 FORGE Weight Tracking — Data Model

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `user_id` | UUID | FK → users |
| `date` | date | One entry per day max. Unique per user+date |
| `weight_kg` | float | 30.0–300.0 kg |
| `source` | enum | `manual`, `apple_health`, `google_fit` (Phase 1: manual only) |
| `created_at` | timestamp | When logged |

### 2.3 Trend Weight Algorithm

**Implement an Exponentially Weighted Moving Average (EWMA):**

```
trend_weight[t] = α × scale_weight[t] + (1 - α) × trend_weight[t-1]
```

Where:
- `α = 0.1` (smoothing factor — gives ~90% weight to history, ~10% to today's reading)
- For missing days: linearly interpolate between known weigh-ins before applying EWMA
- First weigh-in: `trend_weight[0] = scale_weight[0]`
- Minimum 3 weigh-ins before trend is considered reliable

**Why EWMA over simple moving average:**
- SMA treats all days equally. EWMA gives more weight to recent data while still smoothing noise.
- MacroFactor uses something more sophisticated than basic EWMA (proprietary V3 algorithm), but EWMA at α=0.1 gets you 80% of the benefit with 10% of the complexity. Good enough for Phase 2; you can refine later.

**Derived metrics (computed on read, not stored):**

| Metric | Calculation | Display |
|--------|-------------|---------|
| Weekly rate of change | `(trend[today] - trend[7 days ago]) / 7` in kg/day, × 7 for kg/week | "+0.2 kg/week" or "-0.5 kg/week" |
| Monthly rate of change | `(trend[today] - trend[30 days ago]) / 30` × 7 | Smoothed further |
| Goal progress | `(starting_weight - trend[today]) / (starting_weight - goal_weight) × 100` | "43% to goal" |
| Days to goal (estimate) | `remaining_weight / abs(weekly_rate)` | "~8 weeks at current rate" |

### 2.4 Weight Tracking API

```
POST   /api/weight                          → Log today's weight
GET    /api/weight?start=...&end=...        → Raw + trend weights for date range
GET    /api/weight/trend                    → Current trend weight + rate of change + goal progress
DELETE /api/weight/:date                    → Remove a weigh-in
```

### 2.5 Weight Tracking UX

- **Log weight**: accessible from dashboard. Single input field, large number pad optimized for decimal entry (e.g., "82.3"). One tap to save.
- **Weight chart**: full-screen chart showing:
  - Raw scale weight as light dots
  - Trend weight as a bold smooth line
  - Goal weight as a horizontal dashed line
  - Time range toggles: 1W, 1M, 3M, 6M, 1Y, ALL
- **Insight cards below chart**:
  - "Your trend weight is **82.1 kg**, down **0.3 kg** this week"
  - "At this rate, you'll reach **78 kg** in approximately **9 weeks**"
- **Visual indicators**: trend line is green when moving toward goal, red when moving away, gray when flat

---

## 3. Adaptive TDEE & Nutrition Coaching

### 3.1 What MacroFactor Does

MacroFactor's core innovation: instead of relying on a static TDEE formula, it *calculates* your actual energy expenditure from your logged food + weight changes using the CICO equation rearranged:

```
TDEE = Calories_consumed - (Weight_change × Energy_density_of_tissue)
```

Then it adjusts your calorie targets weekly based on this dynamic expenditure estimate.

### 3.2 FORGE Adaptive TDEE — Simplified V1

**After 14+ days of consistent tracking** (weight logged ≥ 10 of last 14 days, nutrition logged ≥ 12 of last 14 days), FORGE computes an adaptive TDEE:

```
average_daily_intake = sum(daily_calories for last 14 days) / 14
weight_change_kg = trend_weight[today] - trend_weight[14 days ago]
weight_change_per_day = weight_change_kg / 14

# Energy density of tissue change
# Assume 70% fat / 30% lean during loss, 40% fat / 60% lean during gain
# Fat ≈ 7700 kcal/kg, Lean ≈ 1800 kcal/kg
if weight_change_per_day < 0:
    energy_density = 0.70 * 7700 + 0.30 * 1800  # = 5930 kcal/kg
else:
    energy_density = 0.40 * 7700 + 0.60 * 1800  # = 4160 kcal/kg

surplus_or_deficit_per_day = weight_change_per_day * energy_density
adaptive_tdee = average_daily_intake - surplus_or_deficit_per_day
```

**Guard rails:**
- Adaptive TDEE clamped to ±40% of formula-based TDEE (from Phase 1 onboarding). This prevents garbage-in-garbage-out when tracking is inconsistent.
- If < 10 weigh-ins in last 14 days OR < 12 nutrition days logged: fall back to formula-based TDEE, show message: "Log more consistently to unlock adaptive targets"
- Recalculate weekly (every Monday), not daily, to avoid over-reactivity
- Show both formula TDEE and adaptive TDEE on the analytics page so user can see the difference

### 3.3 Weekly Check-In

**Inspired by MacroFactor's weekly check-in system:**

Every Monday (configurable), the app:
1. Recalculates adaptive TDEE
2. Compares actual weight change to goal rate of change
3. Adjusts calorie target:
   - If losing faster than goal: increase target by 50–100 kcal
   - If losing slower than goal: decrease target by 50–100 kcal
   - If gaining faster than goal: decrease target by 50–100 kcal
   - If gaining slower than goal: increase target by 50–100 kcal
   - If maintaining and weight stable (±0.1 kg/week): no change
4. Recalculate macro targets based on new calorie target (protein stays fixed at g/kg, fats at g/kg, carbs fill remainder)
5. Present to user: "Your estimated TDEE is **2,650 kcal**. Based on your progress, your new daily target is **2,380 kcal** (previously 2,400). You lost **0.35 kg** last week (goal: 0.4 kg/week)."

**User can:**
- Accept new targets
- Skip check-in (keep current targets for another week)
- Manually override any target

### 3.4 Nutrition Coaching API

```
GET  /api/coaching/status              → Tracking consistency score, adaptive TDEE readiness
GET  /api/coaching/expenditure          → Current adaptive TDEE + formula TDEE + 14/30/90 day history
POST /api/coaching/checkin              → Trigger weekly check-in, returns new recommended targets
PUT  /api/coaching/targets              → Accept or override recommended targets
GET  /api/coaching/checkin/history      → Past check-ins and target adjustments
```

---

## 4. Progress Analytics

### 4.1 Workout Analytics

**Hevy benchmark:** Exercise-specific charts (weight over time, volume over time, estimated 1RM over time), muscle group distribution pie chart, sets per muscle group per week, workout frequency/consistency, PR history.

**FORGE workout analytics — concrete requirements:**

**4.1.1 Exercise Progression Charts**

For each exercise the user has logged ≥ 3 times:

| Chart | X-axis | Y-axis | Data points |
|-------|--------|--------|-------------|
| Weight progression | Date | Max weight used (kg) | One point per workout containing this exercise |
| Volume progression | Date | Total volume (sets × reps × weight) | One point per workout |
| Estimated 1RM | Date | Epley formula: `weight × (1 + reps/30)` using heaviest set | One point per workout |

- Line chart with data points
- Hover/tap to see exact values + workout date
- Time range: 1M, 3M, 6M, 1Y, ALL
- Trend line overlay (linear regression) showing overall direction

**API:**
```
GET /api/analytics/exercise/:exercise_id?range=3m
→ Returns: array of { date, max_weight, total_volume, estimated_1rm }
```

**4.1.2 Muscle Group Distribution**

- Donut/pie chart showing percentage of total weekly volume per muscle group
- Calculated from: sum of (sets × reps × weight) for all exercises in each muscle group for the selected week
- Time range: this week, last week, 4-week average
- **Color coding**: each muscle group has a consistent color across the app
- **Benchmark**: Hevy shows this as "Muscle Distribution" — percentage of training volume per body part

**API:**
```
GET /api/analytics/muscle-distribution?range=4w
→ Returns: array of { muscle_group, volume_kg, percentage }
```

**4.1.3 Weekly Volume per Muscle Group**

- Bar chart: one bar per muscle group, height = total sets targeting that group this week
- Horizontal reference lines at common targets:
  - 10 sets/week (minimum effective volume for most muscle groups)
  - 20 sets/week (maximum recoverable volume for most people)
- This directly answers: "Am I training each muscle group enough?"

**API:**
```
GET /api/analytics/volume-per-muscle?week=2026-W08
→ Returns: array of { muscle_group, total_sets, total_volume_kg }
```

**4.1.4 PR History**

- Chronological list of all personal records
- Each entry: exercise name, weight × reps, date, estimated 1RM
- Filter by exercise, muscle group, or date range
- PR types tracked:
  - **Weight PR**: heaviest weight used for ≥ 1 rep
  - **Volume PR**: highest single-set volume (weight × reps)
  - **1RM PR**: highest estimated 1RM (Epley formula)

**API:**
```
GET /api/analytics/prs?exercise_id=...&type=weight
→ Returns: array of { exercise_name, value, reps, weight, date, pr_type }
```

**4.1.5 Training Consistency**

- Calendar heatmap (GitHub contribution graph style): each day colored by workout status
  - No workout: empty
  - Workout completed: filled (intensity = volume)
- Current streak counter
- Weekly/monthly workout count
- Best streak (all time)

**API:**
```
GET /api/analytics/consistency?year=2026
→ Returns: array of { date, workout_completed, volume_kg }
```

### 4.2 Nutrition Analytics

**MacroFactor benchmark:** Weekly bar charts for each macro, expenditure trend line, energy balance view (intake vs. expenditure over time), nutrient breakdown, consistency/habit tracking.

**FORGE nutrition analytics — concrete requirements:**

**4.2.1 Macro Intake Over Time**

- Stacked bar chart: each day's intake split by protein (blue), carbs (amber), fat (red)
- Target line overlay for calories
- Time range: 1W, 2W, 1M, 3M
- Tap a day → see full breakdown of foods logged that day
- Weekly averages shown below chart: "Avg: 2,340 kcal | 168g P | 245g C | 78g F"

**4.2.2 Calorie Target Adherence**

- Percentage of days within ±10% of calorie target for the selected period
- Displayed as: "You hit your calorie target **78% of days** this month"
- Daily breakdown: color coded (green = within 10%, yellow = within 20%, red = >20% off)

**4.2.3 Expenditure & Energy Balance**

- Line chart: adaptive TDEE over time (once ≥ 14 days data)
- Overlay: daily calorie intake as scatter points
- The gap between the lines visually shows surplus/deficit
- Available only after the adaptive TDEE system activates

**4.2.4 Protein Target Hit Rate**

- For lifters, protein is the most important macro. Separate tracking:
- "You hit ≥ 1.6 g/kg protein on **22 of 30 days** this month"
- Simple percentage ring + daily breakdown

### 4.3 Combined Analytics Dashboard

The analytics page has two tabs: **Training** and **Nutrition**.

**Training tab:**
1. Training consistency heatmap (top)
2. Muscle group distribution donut (left) + volume per muscle bar chart (right)
3. Recent PRs list
4. Exercise selector → individual exercise progression charts

**Nutrition tab:**
1. Macro intake chart (top, default 2-week view)
2. Calorie adherence score + protein hit rate (side by side)
3. Weight trend chart
4. Expenditure trend (if adaptive TDEE active)
5. Weekly check-in history

---

## 5. Progress Photos

### 5.1 Why This Matters

Weight alone doesn't capture body composition changes. MacroFactor supports progress photos from three angles (front, side, back). This is a must-have for any serious fitness app.

### 5.2 Implementation

**Data model:**

| Field | Type | Constraints |
|-------|------|-------------|
| `id` | UUID | PK |
| `user_id` | UUID | FK → users |
| `date` | date | Multiple photos per date allowed (different angles) |
| `angle` | enum | `front`, `side`, `back` |
| `image_path` | string | Path in object storage (MinIO) |
| `weight_kg` | float | Optional — auto-populate from today's weigh-in if available |
| `notes` | string | Optional, max 500 chars |

**Storage:**
- Images stored in MinIO (S3-compatible) under path: `users/{user_id}/progress/{date}_{angle}.jpg`
- Images resized server-side to max 1200px on longest edge (reduce storage + load time)
- Original preserved in a separate bucket for potential future use

**API:**
```
POST   /api/progress-photos              → Upload photo (multipart, specify angle)
GET    /api/progress-photos?start=...&end=...  → List photos in date range
GET    /api/progress-photos/:id           → Get single photo (returns presigned URL)
DELETE /api/progress-photos/:id           → Delete a photo
```

**Frontend UX:**
- Photo gallery: chronological grid view. Tap a date → see all angles from that date
- **Comparison slider**: select two dates, see side-by-side with a slider to swipe between them (front vs front, side vs side). This is the killer feature — being able to visually compare month 1 vs month 3 is incredibly motivating.
- Weight overlay on each photo (if weigh-in exists for that date)
- Reminder prompt: "It's been 2 weeks since your last progress photo" (configurable)

---

## 6. Streaks, Badges & Gamification

### 6.1 Why (Benchmarked Against Strava)

Strava's feedback loop is simple but effective: you exercise, you post, you get kudos. The gamification drives retention. FORGE needs its own version of this, even before the social layer in Phase 3.

### 6.2 Achievement Badges

| Badge | Criteria | Visual |
|-------|----------|--------|
| First Workout | Complete 1 workout | Bronze dumbbell |
| Consistent | 7-day workout streak | Silver calendar |
| Dedicated | 30-day workout streak | Gold calendar |
| Century | Log 100 workouts | Platinum "100" |
| PR Machine | Hit 10 personal records | Gold trophy |
| Macro Master | Hit calorie target ±10% for 7 consecutive days | Green target |
| Protein Pro | Hit protein target ±10% for 14 consecutive days | Blue protein shaker |
| AI Logger | Log 50 meals using food recognition | Camera badge |
| Weight Watcher | Log weight 30 days in a row | Scale badge |
| Body Documenter | Take progress photos 4 weeks in a row | Camera + body badge |

- Badges stored in `user_badges(user_id, badge_id, earned_at)`
- Badge check runs after relevant actions (complete workout, log food, log weight)
- When earned: animated badge reveal on the screen (scale bounce + shimmer effect)
- Viewable on user profile

### 6.3 Streak System

- **Workout streak**: consecutive days with at least one completed workout. Rest days don't break the streak if the user's routine has planned rest days (defined by routine template frequency).
- **Logging streak**: consecutive days with at least one food item logged
- **Weight streak**: consecutive days with a weigh-in
- Streak freeze: user gets 1 free "streak freeze" per week (miss a day without breaking streak). Earned by maintaining a 7+ day streak.

---

## 7. Backend Infrastructure for Phase 2

### 7.1 New Dependencies

| Dependency | Purpose | Integration |
|------------|---------|-------------|
| ONNX Runtime 1.17+ | ML inference | CMake `FetchContent` or prebuilt binaries |
| OpenCV 4.8+ | Image preprocessing | System package or `FetchContent` |
| libcurl | HTTP client for LLM API calls | System package |
| MinIO client (or raw S3 API via libcurl) | Object storage for images | Direct HTTP calls to MinIO |

### 7.2 Async Processing

Food recognition is CPU-intensive. Don't block the HTTP thread.

- Implement a **thread pool** (4–8 threads) for inference tasks
- Request flow: HTTP handler receives image → enqueues preprocessing + inference task → returns `202 Accepted` with `task_id` → frontend polls `GET /api/tasks/:task_id` until complete → returns results
- Alternative: WebSocket push when result is ready (reuse WS infrastructure planned for Phase 3)
- Either approach is fine, but the thread pool itself is mandatory — never run inference on the HTTP serving thread.

### 7.3 Performance Budgets

| Metric | Target | Measurement |
|--------|--------|-------------|
| Image preprocessing | < 50ms p95 | Server-side timer |
| Tier 1 inference (ONNX) | < 100ms p95 on CPU | Server-side timer |
| Full recognition (Tier 1 only) | < 500ms p95 | End-to-end |
| Full recognition (Tier 1 + Tier 2) | < 3000ms p95 | End-to-end |
| Trend weight computation (1 year of data) | < 10ms | Server-side timer |
| Analytics queries (exercise progression, 1 year) | < 200ms p95 | Server-side timer |
| Progress photo upload | < 2000ms p95 | Including resize + storage write |

---

## 8. Definition of Done

Phase 2 is complete when **all** of the following are true:

- [ ] User can take a photo of food and receive AI-identified food items with estimated macros
- [ ] AI results are presented as editable cards that can be adjusted before logging
- [ ] Confidence indicator shows how certain the AI is about each food item
- [ ] Fallback to manual search works seamlessly when AI misidentifies food
- [ ] ONNX Runtime inference runs in < 100ms on CPU with zero memory leaks
- [ ] User can log daily weight and see a smooth trend line that filters out noise
- [ ] Weight trend chart shows raw scale weight (dots) vs trend weight (line) vs goal (dashed)
- [ ] Adaptive TDEE activates after 14 days of consistent tracking and shows on analytics
- [ ] Weekly check-in proposes adjusted calorie/macro targets based on actual progress
- [ ] Exercise progression charts show weight, volume, and estimated 1RM over time
- [ ] Muscle group distribution and weekly volume charts are functional
- [ ] PR history page lists all personal records chronologically
- [ ] Training consistency heatmap renders for the current year
- [ ] Nutrition analytics show daily macro intake, calorie adherence, and protein hit rate
- [ ] User can upload progress photos from three angles and compare two dates side-by-side
- [ ] Achievement badges unlock and display with animation
- [ ] All inference tasks run on a background thread pool — never on the HTTP thread
- [ ] 1000 sequential recognition requests complete under ASan with zero leaks

**The test scenario: You eat lunch, snap a photo, the app identifies "chicken breast, rice, broccoli" in < 3 seconds. You adjust the rice portion from 150g to 200g, tap Log. On the analytics page, you see your weight trend is down 0.4 kg this week, your TDEE is calculated at 2,580 kcal, and your bench press 1RM has gone up 5 kg over the last month. You check your progress photos and slider-compare today vs. 8 weeks ago. You earned the "Macro Master" badge yesterday. Phase 2 is done.**
-e 
---

# 6. Phase 3: Community & Social

## Detailed Functional Specification

> **Benchmark apps:** Strava (social feed, kudos, clubs — 180M users, $5.7M/mo), Hevy (workout feed, routine sharing, likes/comments — ~10M users), Boostcamp (program marketplace with 70+ coach programs + 2,000+ community programs), JEFIT (8M+ users, community workout library)
>
> **Phase 3 goal:** Transform FORGE from a solo tracking tool into a social fitness platform. Users follow friends, see each other's workouts in a feed, react and comment, browse and publish community workout programs, join clubs, and message each other. The social layer is the retention engine — research shows that Strava users who receive kudos exercise more frequently, and club-affiliated athletes are 2× more likely to log weekly activity.

---

## 1. Social Graph

### 1.1 Follow Model

**What the best apps do:** Strava and Hevy both use an asymmetric follow model (like Instagram), not a symmetric friend model (like Facebook). This is correct — it allows users to follow athletes and coaches without mutual consent, enabling content discovery and aspiration-driven engagement.

**Concrete requirements:**

- `POST /api/social/follow` — follow a user by `user_id`
- `DELETE /api/social/follow/:user_id` — unfollow
- `GET /api/social/followers?user_id=&cursor=&limit=20` — paginated followers list
- `GET /api/social/following?user_id=&cursor=&limit=20` — paginated following list
- `GET /api/social/follow-status/:user_id` — returns `{ is_following: bool, is_followed_by: bool }`

**Data model:**

```sql
CREATE TABLE follows (
    follower_id UUID NOT NULL REFERENCES users(id),
    followed_id UUID NOT NULL REFERENCES users(id),
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY (follower_id, followed_id),
    CHECK (follower_id != followed_id)
);
CREATE INDEX idx_follows_followed ON follows(followed_id, created_at DESC);
CREATE INDEX idx_follows_follower ON follows(follower_id, created_at DESC);
```

**Counts:** Denormalized `follower_count` and `following_count` on `users` table, updated via trigger or app-level increment/decrement. Never compute with `COUNT(*)` at read time.

**Rate limit:** Max 100 follow/unfollow actions per hour per user.

### 1.2 User Profiles

**Hevy benchmark:** Profile shows avatar, display name, bio, follower/following counts, total workouts, total volume, current streak, chronological workout list. You can tap any workout to see details and "Save as Routine."

**Strava benchmark:** Profile shows training stats prominently. Mutual followers shown. Activity calendar (GitHub contribution graph) shows consistency.

**Concrete requirements:**

- `GET /api/users/:username` — public profile endpoint
- Response: `display_name`, `username`, `avatar_url`, `bio` (max 160 chars), `follower_count`, `following_count`, `total_workouts`, `total_volume_kg`, `current_streak_days`, `member_since`, `is_following`, `is_followed_by`, `badges[]`
- **Privacy levels** (user-configurable):
  - `public` — anyone sees profile + workouts
  - `followers_only` — only approved followers see workouts (follow requests require approval)
  - `private` — profile visible but no workouts shown
- Default: `public` (maximizes network effects for early growth)
- Avatar: max 2MB, JPEG/PNG, resized server-side to 256×256, stored in MinIO `avatars/{user_id}.jpg`

### 1.3 User Discovery

- `GET /api/social/search?q=&limit=20` — search users by username or display_name. Full-text with trigram matching (`pg_trgm`)
- `GET /api/social/suggestions?limit=10` — suggested users. Algorithm:
  1. Users followed by people you follow (friends-of-friends), ranked by overlap count
  2. Users with similar workout patterns (same muscle groups, similar volume)
  3. Exclude already-followed and blocked users
- Suggestions refresh daily (cache, not computed per request)
- Response time: <200ms p95

### 1.4 Blocking & Reporting

- `POST /api/social/block/:user_id` — block (also unfollows both directions)
- `DELETE /api/social/block/:user_id` — unblock
- `POST /api/social/report` — report user or content. Body: `{ target_type: "user"|"workout"|"comment"|"program", target_id, reason, details? }`
- Reasons: `spam`, `harassment`, `inappropriate_content`, `impersonation`, `other`
- Blocked users can't see your profile, workouts, or message you

---

## 2. Activity Feed

### 2.1 Feed Design

**Strava:** Reverse-chronological feed of activities from followed users. Activities show user, title, stats, map, photos, kudos count, comments. Activities with PRs get 3× more engagement.

**Hevy:** Feed shows workouts with user, name, description, duration, volume, PR count, exercise summary (first 3–4 exercises with sets×reps×weight), likes, comments. Users can "Save as Routine" or "Copy Workout" directly from feed.

**FORGE feed card content:**

```
┌──────────────────────────────────────────┐
│ [Avatar] Display Name            2h ago  │
│                                          │
│ 💪 Upper Body Push                       │
│ "Finally hit 100kg bench!"               │
│                                          │
│ ⏱ 58 min  📊 12,450 kg  🏆 2 PRs        │
│                                          │
│ Bench Press       4×8 @ 80kg             │
│ OHP               3×10 @ 45kg            │
│ Cable Flyes       3×12 @ 15kg            │
│ +2 more exercises                        │
│                                          │
│ [📸 Photo/Video if attached]             │
│                                          │
│ 🔥 12   💬 3          ↗ Share   •••      │
└──────────────────────────────────────────┘
```

**Feed item actions:**
- **React**: tap 🔥 to react (single reaction type, like Strava kudos — low friction)
- **Comment**: tap 💬 to open comment thread
- **Share**: generate shareable image (Hevy-style shareables — workout summary with stats)
- **•••** menu: "Save as Routine", "Copy Workout", "Report", "Mute user"

### 2.2 Feed Architecture

**Fan-out-on-read (pull model)** for initial scale (<100K users):

```sql
SELECT w.*, u.username, u.display_name, u.avatar_url,
       (SELECT COUNT(*) FROM reactions r WHERE r.workout_id = w.id) as reactions_count,
       (SELECT COUNT(*) FROM comments c WHERE c.workout_id = w.id) as comments_count
FROM workouts w
JOIN users u ON w.user_id = u.id
WHERE w.user_id IN (SELECT followed_id FROM follows WHERE follower_id = :current_user)
  AND w.status = 'completed'
  AND w.visibility IN ('public', 'followers')
  AND w.deleted_at IS NULL
ORDER BY w.completed_at DESC
LIMIT 20;
```

**Key index:**
```sql
CREATE INDEX idx_workouts_feed ON workouts(user_id, completed_at DESC)
WHERE status = 'completed' AND deleted_at IS NULL;
```

**Cursor pagination:** Use `completed_at` as cursor, not OFFSET. Client sends `?before=2026-02-19T14:30:00Z&limit=20`.

**Discover tab:** Same query but without the follow filter — shows recent public workouts from all users. Useful for new users with empty feed.

### 2.3 Feed API

```
GET  /api/feed?before=&limit=20         → Following feed (paginated)
GET  /api/feed/discover?before=&limit=20 → Public discover feed
POST /api/feed/hide/:workout_id          → Hide a specific workout from your feed
```

### 2.4 Workout Visibility

When completing a workout, user chooses visibility:
- `public` — appears in all feeds and discover
- `followers` — only followers see it
- `private` — only user sees it (not in any feed)

Default: `public`. Configurable as a global default in settings.

### 2.5 Real-time Feed Updates

When a user you follow completes a workout:
1. Backend publishes event to a WebSocket channel
2. If the follower's feed page is open, a "New workout from [User]" banner slides down at top of feed
3. Tapping the banner scrolls to top and loads the new workout

Implementation: Each user subscribes to a WebSocket channel `feed:{user_id}`. When workout completes, backend publishes to all followers' channels.

---

## 3. Reactions & Comments

### 3.1 Reactions

**Strava's "Kudos" insight:** Kudos is a single-tap, no-text-required reaction. This is deliberately low friction — it removes the cognitive load of "what do I say?" Research shows that receiving kudos correlates with increased exercise frequency.

**FORGE reactions:**
- Single reaction type: 🔥 (fire). One tap to give/remove. No emoji picker, no multiple types. Simplicity = higher engagement.
- `POST /api/workouts/:id/react` — toggle reaction on/off
- `GET /api/workouts/:id/reactions?limit=50` — list of users who reacted
- Reaction count denormalized on workout row for fast feed rendering

**Data model:**
```sql
CREATE TABLE reactions (
    user_id    UUID NOT NULL REFERENCES users(id),
    workout_id UUID NOT NULL REFERENCES workouts(id),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY (user_id, workout_id)
);
```

### 3.2 Comments

- `POST /api/workouts/:id/comments` — add comment (body: `{ text: string, parent_id?: uuid }`)
- `GET /api/workouts/:id/comments?limit=50` — threaded comments
- `DELETE /api/comments/:id` — delete own comment
- Comment max length: 500 chars
- Support reply threading (one level deep — no infinite nesting)
- Clickable @mentions: type `@username` → autocomplete → creates notification for mentioned user

**Data model:**
```sql
CREATE TABLE comments (
    id         UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workout_id UUID NOT NULL REFERENCES workouts(id),
    user_id    UUID NOT NULL REFERENCES users(id),
    parent_id  UUID REFERENCES comments(id),
    body       TEXT NOT NULL CHECK (length(body) BETWEEN 1 AND 500),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    deleted_at TIMESTAMPTZ
);
CREATE INDEX idx_comments_workout ON comments(workout_id, created_at);
```

---

## 4. Notifications

### 4.1 Notification Types

| Type | Trigger | Text |
|------|---------|------|
| `reaction` | Someone reacts to your workout | "[User] 🔥'd your Push Day workout" |
| `comment` | Someone comments on your workout | "[User] commented on your Push Day" |
| `mention` | Someone @mentions you | "[User] mentioned you in a comment" |
| `follow` | Someone follows you | "[User] started following you" |
| `follow_request` | Someone requests to follow (if followers_only) | "[User] wants to follow you" |
| `pr_celebration` | Friend hits a PR | "[User] hit a new bench press PR! 🏆" |
| `program_upvote` | Someone upvotes your published program | "[User] upvoted your PPL Split" |
| `weekly_checkin` | Weekly coaching check-in ready | "Your weekly check-in is ready" |
| `streak_reminder` | About to lose a streak | "Don't break your 14-day streak! 💪" |
| `badge_earned` | User earns a badge | "You earned the Dedicated badge! 🎖️" |

### 4.2 Notification API & Storage

```
GET  /api/notifications?cursor=&limit=20  → Paginated notifications
POST /api/notifications/read              → Mark all as read
POST /api/notifications/:id/read          → Mark specific as read
GET  /api/notifications/unread-count      → Number of unread notifications (for badge)
```

**Data model:**
```sql
CREATE TABLE notifications (
    id          UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id     UUID NOT NULL REFERENCES users(id),
    type        TEXT NOT NULL,
    actor_id    UUID REFERENCES users(id),
    target_type TEXT,
    target_id   UUID,
    read_at     TIMESTAMPTZ,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
CREATE INDEX idx_notifications_user ON notifications(user_id, created_at DESC);
CREATE INDEX idx_notifications_unread ON notifications(user_id) WHERE read_at IS NULL;
```

### 4.3 Real-time Notifications

- WebSocket push for new notifications
- When notification created: push to user's WS channel `notifications:{user_id}`
- Frontend shows notification badge count in real time
- Desktop: browser notification permission (optional)

---

## 5. Community Workout Programs (Marketplace)

### 5.1 What Exists Today

**Boostcamp:** 70+ expert-created programs (Greg Nuckols, Eric Helms, etc.) + 2,000+ community programs. Programs are multi-week with structured progression. Free programs + premium programs.

**Hevy:** Users can share routines via link. Other users can "Save as Routine." But there's no browsable marketplace — sharing is manual (links, Reddit posts).

**JEFIT:** 8M+ users, large library of community-created programs browsable by category. Older UX but proves the concept works.

**FORGE goes further:** A browsable, upvotable, filterable marketplace for community workout programs — like Reddit meets Boostcamp.

### 5.2 Program Data Model

```sql
CREATE TABLE programs (
    id             UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    author_id      UUID NOT NULL REFERENCES users(id),
    title          TEXT NOT NULL CHECK (length(title) BETWEEN 3 AND 100),
    description    TEXT CHECK (length(description) <= 2000),
    category       TEXT NOT NULL, -- 'strength', 'hypertrophy', 'powerlifting', 'calisthenics', 'conditioning', 'sport_specific', 'general_fitness'
    difficulty      TEXT NOT NULL, -- 'beginner', 'intermediate', 'advanced'
    days_per_week  INT NOT NULL CHECK (days_per_week BETWEEN 1 AND 7),
    duration_weeks INT, -- null = ongoing
    equipment      TEXT[], -- array: ['barbell', 'dumbbell', 'cable', 'machine', ...]
    tags           TEXT[], -- ['ppl', '5x5', 'upper_lower', 'full_body', 'bro_split', ...]
    upvote_count   INT NOT NULL DEFAULT 0,
    save_count     INT NOT NULL DEFAULT 0,
    visibility     TEXT NOT NULL DEFAULT 'public', -- 'public', 'unlisted', 'private'
    created_at     TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at     TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    deleted_at     TIMESTAMPTZ
);
CREATE INDEX idx_programs_category ON programs(category, upvote_count DESC) WHERE deleted_at IS NULL;
CREATE INDEX idx_programs_author ON programs(author_id);

CREATE TABLE program_workouts (
    id          UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    program_id  UUID NOT NULL REFERENCES programs(id),
    day_number  INT NOT NULL, -- day 1, day 2, etc.
    week_number INT NOT NULL DEFAULT 1, -- for multi-week programs
    name        TEXT NOT NULL, -- "Push Day", "Leg Day A", etc.
    notes       TEXT,
    sort_order  INT NOT NULL
);

CREATE TABLE program_exercises (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    program_workout_id  UUID NOT NULL REFERENCES program_workouts(id),
    exercise_id         UUID NOT NULL REFERENCES exercises(id),
    sort_order          INT NOT NULL,
    sets                INT NOT NULL,
    rep_range_low       INT, -- e.g., 8
    rep_range_high      INT, -- e.g., 12 (so "8-12 reps")
    rpe_target          FLOAT, -- optional
    rest_seconds        INT, -- recommended rest
    notes               TEXT, -- "Use controlled tempo", "Superset with next"
    superset_group      INT -- null = standalone, 1/2/3 = superset grouping
);
```

### 5.3 Program Marketplace API

```
POST   /api/programs                         → Publish a program
GET    /api/programs/:id                     → Get full program with all workouts/exercises
PUT    /api/programs/:id                     → Edit own program
DELETE /api/programs/:id                     → Delete own program

GET    /api/programs/browse?category=&difficulty=&days_per_week=&sort=popular&page=1
                                             → Browse marketplace with filters
GET    /api/programs/search?q=ppl+beginner   → Full-text search
GET    /api/programs/trending                → Top programs by recent upvotes (last 7 days)

POST   /api/programs/:id/upvote              → Toggle upvote
POST   /api/programs/:id/save                → Save program to user's library
POST   /api/programs/:id/start               → Start following the program (creates routines from it)

GET    /api/users/:username/programs         → Programs published by a user
GET    /api/programs/saved                   → User's saved programs
```

### 5.4 Program Marketplace UX

**Browse page layout:**
- Top: filter bar — Category (dropdown), Difficulty (pills), Days/Week (pills), Equipment (multi-select), Sort (Popular / Newest / Most Saved)
- Program cards in a grid:
  ```
  ┌──────────────────────────────────────┐
  │ PPL Hypertrophy Split                │
  │ by @stronglifter42                   │
  │                                      │
  │ 🏋️ Hypertrophy  ⭐ Intermediate      │
  │ 📅 6 days/week  ⏱ 12 weeks          │
  │ Equipment: Barbell, Dumbbell, Cable  │
  │                                      │
  │ 🔼 847 upvotes   💾 1,203 saves      │
  └──────────────────────────────────────┘
  ```
- Tap card → full program detail page with week-by-week breakdown, all exercises listed, author profile, reviews/comments

**Publishing flow:**
1. User creates routines in their Workout tab (they already have this from Phase 1)
2. "Publish as Program" button — select routines to include, set order, add title/description/category/difficulty
3. Program is live in marketplace immediately
4. Author sees analytics: views, upvotes, saves, how many users are currently following

### 5.5 Upvote System

- `POST /api/programs/:id/upvote` — toggle (like Reddit — one upvote per user per program)
- Upvotes stored in `program_upvotes(user_id, program_id, created_at)` table
- `upvote_count` denormalized on `programs` table
- Trending algorithm: upvotes in last 7 days, weighted by recency (Wilson score or similar)

---

## 6. Messaging

### 6.1 Design Decisions

**No fitness app does messaging well.** Strava has no DMs. Hevy has no DMs. This is a gap.

FORGE implements lightweight 1-on-1 messaging — not trying to be Discord or WhatsApp, just enough for gym-related conversation.

### 6.2 Data Model

```sql
CREATE TABLE conversations (
    id           UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at   TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE conversation_participants (
    conversation_id UUID NOT NULL REFERENCES conversations(id),
    user_id         UUID NOT NULL REFERENCES users(id),
    last_read_at    TIMESTAMPTZ,
    PRIMARY KEY (conversation_id, user_id)
);

CREATE TABLE messages (
    id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    conversation_id UUID NOT NULL REFERENCES conversations(id),
    sender_id       UUID NOT NULL REFERENCES users(id),
    body            TEXT NOT NULL CHECK (length(body) BETWEEN 1 AND 2000),
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    deleted_at      TIMESTAMPTZ
);
CREATE INDEX idx_messages_conversation ON messages(conversation_id, created_at DESC);
```

### 6.3 Messaging API

```
GET    /api/conversations                            → List conversations (with latest message preview, unread count)
POST   /api/conversations                            → Start conversation (body: { user_id })
GET    /api/conversations/:id/messages?before=&limit=50  → Messages (paginated, newest first)
POST   /api/conversations/:id/messages               → Send message (body: { text })
POST   /api/conversations/:id/read                   → Mark conversation as read
DELETE /api/messages/:id                              → Delete own message
```

### 6.4 Real-time Messaging

- WebSocket channel per conversation: `chat:{conversation_id}`
- On send: message saved to DB → published to WS channel → all participants receive instantly
- Typing indicator: client sends `typing` event on WS, other participants see "typing..." for 3 seconds
- Unread count updates in real-time via notification WS channel
- Message delivery: at-least-once semantics. Client deduplicates by message ID.

### 6.5 Messaging UX

- Conversation list: sorted by most recent message, shows avatar, name, last message preview, unread badge
- Chat view: messages in reverse chronological scroll (newest at bottom), auto-scroll on new message
- Can share a workout to a conversation: "Share Workout" from workout detail → select conversation → sends a workout card (interactive, tappable)
- Can share a program: similar to workout sharing
- Rate limit: 60 messages per hour per user (anti-spam)
- Blocked users cannot initiate conversations or send messages

---

## 7. Clubs

### 7.1 Why Clubs Matter

**Strava insight:** 1M+ clubs exist on Strava. Club-affiliated athletes are 2× more likely to log weekly activity. Gen Z users cite "sense of belonging" as top reason for joining. Clubs create accountability and real-world community.

### 7.2 Data Model

```sql
CREATE TABLE clubs (
    id           UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name         TEXT NOT NULL CHECK (length(name) BETWEEN 3 AND 100),
    description  TEXT CHECK (length(description) <= 1000),
    avatar_url   TEXT,
    owner_id     UUID NOT NULL REFERENCES users(id),
    privacy      TEXT NOT NULL DEFAULT 'public', -- 'public', 'private'
    member_count INT NOT NULL DEFAULT 0,
    created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE club_memberships (
    club_id    UUID NOT NULL REFERENCES clubs(id),
    user_id    UUID NOT NULL REFERENCES users(id),
    role       TEXT NOT NULL DEFAULT 'member', -- 'owner', 'admin', 'member'
    joined_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY (club_id, user_id)
);
```

### 7.3 Club API

```
POST   /api/clubs                          → Create club
GET    /api/clubs/:id                      → Club details + member count + recent activity
PUT    /api/clubs/:id                      → Edit club (owner/admin only)
DELETE /api/clubs/:id                      → Delete club (owner only)

POST   /api/clubs/:id/join                 → Join club
DELETE /api/clubs/:id/leave                → Leave club
GET    /api/clubs/:id/members?page=1       → List members

GET    /api/clubs/:id/feed?before=&limit=20 → Club activity feed (workouts from members)
GET    /api/clubs/browse?q=&page=1         → Browse/search clubs
GET    /api/clubs/mine                     → User's clubs
```

### 7.4 Club Features

- **Club feed**: separate feed showing workouts from club members only
- **Club leaderboard**: weekly volume, workout count, streak leaders within the club
- **Club challenges**: owner can create time-limited challenges (e.g., "Most volume this week", "Longest streak in February")
- Max 10 clubs per user (prevent spam)
- Max 500 members per club (for Phase 3 — can scale later)

---

## 8. Social Media Shareables

**Hevy benchmark:** After completing a workout, Hevy generates beautiful shareable images — workout summary with stats, PR celebrations, monthly reports. Users post these on Instagram stories, Reddit, etc.

### 8.1 FORGE Shareables

After workout completion, generate shareable image cards:

| Shareable Type | Content |
|---------------|---------|
| Workout summary | Exercise list, volume, duration, PRs — branded with FORGE logo |
| PR celebration | Exercise name, new weight/reps, previous best, improvement % |
| Monthly report | Workouts completed, total volume, PRs hit, streak, consistency % |
| Year in review | Total workouts, volume, PRs, most-trained muscle group, longest streak |

- Generated server-side using a headless HTML → image renderer (e.g., Puppeteer/wkhtmltoimage called from C++) or client-side using HTML Canvas
- Saved temporarily and served via presigned MinIO URL
- Include FORGE branding (logo watermark, "Tracked with FORGE" footer)
- User can save image to gallery or share directly to Instagram/Twitter/WhatsApp

---

## 9. WebSocket Infrastructure

### 9.1 Architecture

Phase 3 requires real-time features: feed updates, notifications, messaging. All served via WebSocket.

**Connection flow:**
1. Client connects to `wss://api.forge.app/ws` with JWT as query param or header
2. Server authenticates, creates connection entry
3. Server subscribes connection to channels: `feed:{user_id}`, `notifications:{user_id}`, `chat:{conversation_id}` for each active conversation
4. Server pushes events as JSON: `{ "channel": "...", "event": "new_workout", "data": {...} }`

**C++ implementation:**
- Crow and Drogon both have WebSocket support built-in
- Maintain a connection map: `user_id → std::vector<WebSocket*>` (user may have multiple tabs/devices)
- When broadcasting to followers: iterate `follows` table, look up connections, send. For <100K users this is fine.
- Heartbeat: server sends ping every 30s, client responds with pong. Close connection after 3 missed pongs.

### 9.2 WebSocket API

**Client → Server messages:**
```json
{ "action": "subscribe", "channel": "chat:abc123" }
{ "action": "unsubscribe", "channel": "chat:abc123" }
{ "action": "typing", "conversation_id": "abc123" }
{ "action": "ping" }
```

**Server → Client messages:**
```json
{ "channel": "feed:user123", "event": "new_workout", "data": { "workout_id": "...", "user": {...} } }
{ "channel": "notifications:user123", "event": "new_notification", "data": { "type": "reaction", ... } }
{ "channel": "chat:conv456", "event": "new_message", "data": { "id": "...", "text": "...", "sender": {...} } }
{ "channel": "chat:conv456", "event": "typing", "data": { "user_id": "...", "username": "..." } }
```

---

## 10. Phase 3 Performance Budgets

| Metric | Target |
|--------|--------|
| Feed query (20 items) | <200ms p95 |
| Reaction toggle | <100ms p95 |
| Comment post | <150ms p95 |
| Notification fetch (20 items) | <100ms p95 |
| WebSocket message delivery | <200ms p95 (server to client) |
| Program browse with filters | <300ms p95 |
| Program search (full-text) | <500ms p95 |
| Message send (WebSocket) | <100ms p95 |
| User search | <200ms p95 |

---

## 11. Phase 3 Definition of Done

- [ ] User can follow/unfollow other users
- [ ] Activity feed shows workouts from followed users in reverse chronological order
- [ ] Discover tab shows recent public workouts from all users
- [ ] User can react (🔥) to workouts with one tap
- [ ] User can comment on workouts with @mention support
- [ ] Notifications appear in real-time via WebSocket
- [ ] User can publish workout routines as community programs
- [ ] Program marketplace is browsable with filters (category, difficulty, days/week)
- [ ] Programs can be upvoted and saved
- [ ] User can start a saved program (creates routines from it)
- [ ] Trending programs surface recent popular content
- [ ] 1-on-1 messaging works with real-time delivery
- [ ] Typing indicators work in chat
- [ ] Workouts can be shared as image cards
- [ ] Clubs can be created, joined, and have their own feed
- [ ] User profiles show public stats, badges, and workout history
- [ ] Privacy settings (public/followers/private) work correctly
- [ ] Blocked users are hidden from all social features
- [ ] WebSocket connections are stable, reconnect automatically on disconnect

**The test scenario: You finish your workout and it appears in your followers' feeds within 2 seconds. Your friend Alice reacts 🔥 and comments "Nice PR!" — you get a real-time notification. You browse the marketplace, find a PPL program with 500 upvotes, save it and start it tomorrow. You message your gym buddy Bob "leg day tomorrow?" and he responds instantly. You create a "Zürich Lifters" club and invite friends. Phase 3 is done.**

---

# 7. Phase 4: Polish & Scale

## Detailed Functional Specification

> **Phase 4 goal:** Make FORGE production-ready. Offline support, PWA installability, performance optimization, rate limiting, caching, data export, accessibility, and onboarding refinement. This phase turns a working prototype into a product people can rely on daily.

---

## 1. Progressive Web App (PWA)

### 1.1 Why PWA

FORGE is a web app that needs to feel native. Users will open it at the gym between sets — it must load instantly and work even with poor gym WiFi.

### 1.2 Requirements

- **Service Worker**: precache all static assets (JS bundles, CSS, fonts, images). Runtime cache API responses with stale-while-revalidate strategy.
- **Web App Manifest**: `manifest.json` with app name, icons (192px, 512px), `display: standalone`, theme color matching FORGE's dark theme
- **Install prompt**: after 3rd visit, show "Add to Home Screen" banner. Never show on first visit.
- **Offline indicator**: when device is offline, show subtle banner at top: "You're offline — data will sync when connected"
- **Splash screen**: FORGE logo on dark background during PWA launch

### 1.3 Lighthouse Targets

| Audit | Target |
|-------|--------|
| Performance | ≥ 90 |
| Accessibility | ≥ 90 |
| Best Practices | ≥ 95 |
| SEO | ≥ 90 |
| PWA | ✅ All checks pass |

---

## 2. Offline Support

### 2.1 Offline Workout Logging

The most critical offline feature. Users are in a gym basement with no signal.

**Implementation:**
- When user starts a workout, all data is stored in IndexedDB locally first
- Each set logged writes to IndexedDB immediately
- If online: sync to server in real-time (optimistic update pattern)
- If offline: queue all writes in IndexedDB. Show "Offline — will sync later" indicator.
- When connection restored: replay queued operations to server in order, handle conflicts by timestamp (last-write-wins for sets, server-authoritative for computed fields like PRs)

**Conflict resolution:**
- Exercise sets: client-wins (user's logged data is ground truth)
- PRs: server recomputes after sync
- Nutrition logs: client-wins
- Social actions (reactions, comments): server-wins (discard offline reactions if workout was deleted)

### 2.2 Offline Data Available

| Data | Offline availability |
|------|---------------------|
| User profile & targets | Cached in IndexedDB |
| Exercise library (200+) | Prefetched on first load |
| Workout routines | Cached in IndexedDB |
| Recent 50 foods | Cached in IndexedDB |
| Today's nutrition log | Cached in IndexedDB |
| Workout history (last 30) | Cached in IndexedDB |
| Social feed | NOT available offline (acceptable) |
| Messaging | NOT available offline |
| AI food recognition | NOT available offline |

### 2.3 Sync Engine

```typescript
// Conceptual sync queue
interface SyncOperation {
  id: string;
  endpoint: string;
  method: 'POST' | 'PUT' | 'DELETE';
  body: any;
  timestamp: number;
  retries: number;
}

// On reconnect:
// 1. Sort queue by timestamp
// 2. Replay operations sequentially
// 3. Remove from queue on 2xx response
// 4. Retry on 5xx (max 3 times)
// 5. Alert user on 4xx (data conflict)
```

---

## 3. Performance Optimization

### 3.1 Backend Caching

**Redis or in-memory cache for hot paths:**

| Endpoint | Cache strategy | TTL |
|----------|---------------|-----|
| `GET /api/dashboard` | Cache per user, invalidate on write | 60s |
| `GET /api/feed` | Cache per user, invalidate on new followed workout | 30s |
| `GET /api/programs/trending` | Global cache | 5min |
| `GET /api/programs/browse` | Cache per filter combo | 2min |
| `GET /api/foods/search` (USDA) | Cache per query | 30 days |
| `GET /api/notifications/unread-count` | Cache per user | 10s |
| `GET /api/analytics/*` | Cache per user+range | 5min |

**Implementation**: Start with PostgreSQL-backed cache (materialized views + application-level `std::unordered_map` with TTL). Graduate to Redis when memory becomes an issue.

### 3.2 Database Optimization

- **Connection pooling**: libpqxx connection pool with min 5, max 20 connections
- **Prepared statements**: all frequent queries use prepared statements (avoid parse overhead)
- **Partial indexes**: all queries on soft-deleted tables use `WHERE deleted_at IS NULL` partial index
- **EXPLAIN ANALYZE**: run on all queries returning >10ms. No full table scans on tables >10K rows
- **Vacuuming**: auto-vacuum configured with aggressive settings for high-write tables (exercise_sets, nutrition_log)
- **Read replicas**: NOT needed for <100K users. Document when to add.

### 3.3 Frontend Optimization

- **Code splitting**: React.lazy() for routes. Dashboard, Workout, Nutrition, Social, Analytics, Settings are separate chunks.
- **Image optimization**: all user-uploaded images served via MinIO with resizing (thumbnail 100px, preview 400px, full 1200px). Use `<img srcset>` for responsive loading.
- **Virtualized lists**: workout history and feed use windowed rendering (`react-window` or `@tanstack/virtual`) — only render visible items
- **Bundle budget**: < 500KB gzipped total. < 200KB for initial route. Monitor with `vite-plugin-visualizer`.
- **Font optimization**: Inter + JetBrains Mono loaded via `font-display: swap` with WOFF2 subset

### 3.4 API Response Compression

- All API responses > 1KB compressed with gzip (or brotli if client supports)
- Static assets served with `Cache-Control: public, max-age=31536000, immutable` (hashed filenames)
- API responses: `Cache-Control: private, no-cache` with ETag support for conditional requests

---

## 4. Rate Limiting

### 4.1 Global Rate Limits

| Endpoint group | Limit | Window |
|---------------|-------|--------|
| Auth (login/register) | 5 requests | 15 minutes per IP |
| API (authenticated) | 300 requests | 1 minute per user |
| AI recognition | 20 requests | 1 hour per user |
| LLM calls (Tier 2) | 100 requests | 24 hours per user |
| Social actions (follow/react) | 100 actions | 1 hour per user |
| Messages | 60 messages | 1 hour per user |
| File uploads | 20 uploads | 1 hour per user |

### 4.2 Implementation

- Token bucket algorithm per user
- Store bucket state in memory (or Redis)
- Return `429 Too Many Requests` with `Retry-After` header
- Separate limits for expensive operations (AI, uploads) vs. cheap reads

---

## 5. Data Export

### 5.1 Export Formats

Users own their data. They must be able to export everything.

```
GET /api/export/workouts?format=csv|json&start=&end=
GET /api/export/nutrition?format=csv|json&start=&end=
GET /api/export/weight?format=csv|json
GET /api/export/all → ZIP containing all data as JSON files
```

### 5.2 CSV Format (Workout Example)

```csv
date,workout_name,exercise,set_number,set_type,weight_kg,reps,rpe,rest_seconds,is_pr
2026-02-19,Push Day,Bench Press,1,warmup,40,10,,90,false
2026-02-19,Push Day,Bench Press,2,working,80,8,8.0,120,false
2026-02-19,Push Day,Bench Press,3,working,80,8,8.5,120,false
2026-02-19,Push Day,Bench Press,4,working,80,7,9.0,120,true
```

### 5.3 Import Support

- Import from CSV (FORGE format)
- Import from Strong (CSV export)
- Import from Hevy (CSV export)
- Import from MyFitnessPal (CSV export — nutrition only)
- Each import has a dedicated parser that maps fields to FORGE schema

---

## 6. Onboarding Refinement

### 6.1 First-Time User Experience

The app's first 60 seconds determine if a user stays. Refine the Phase 1 onboarding:

1. **Welcome screen**: "Welcome to FORGE. Track workouts. Log nutrition. Join the community." — one CTA: "Get Started"
2. **Profile setup**: Same 5-screen wizard from Phase 1, but now with progress dots and back navigation
3. **First workout prompt**: After onboarding, instead of dumping user on empty dashboard, show: "Start your first workout" with 3 pre-made beginner routines to choose from
4. **First food log prompt**: After first workout, suggest: "Log your post-workout meal" with a quick tutorial overlay
5. **Social prompt**: After first food log: "Follow some athletes for inspiration" with suggested users carousel
6. **Tutorial overlays**: first time a user enters each major screen, show a brief tooltip pointing to key features (dismissible, only shows once)

### 6.2 Empty States

Every list/feed has a thoughtful empty state:

| Screen | Empty state message |
|--------|-------------------|
| Dashboard (no workouts) | "Start your first workout to see your progress here" + Start Workout CTA |
| Workout history | "Your workout history will appear here. Start your first session!" |
| Food log (today) | "Log your meals to track macros and calories" + Quick Add CTA |
| Feed (no follows) | "Follow athletes to see their workouts here" + Suggested users |
| Notifications | "You're all caught up! 🎉" |
| Programs (none saved) | "Browse the marketplace to find a training program" + Browse CTA |
| Messages (none) | "Message your gym friends" |

---

## 7. Security Hardening

### 7.1 Input Validation

- All string inputs sanitized: strip HTML tags, limit length, validate character sets
- SQL injection: impossible with parameterized queries (libpqxx `$1` params). Never concatenate user input into SQL.
- XSS: all user-generated content HTML-escaped before rendering. React handles this by default for JSX interpolation, but verify for `dangerouslySetInnerHTML` (should never be used with user content).
- CSRF: not applicable for JWT-based API (no cookies for auth — except refresh token). Refresh endpoint validates HttpOnly cookie + checks Origin header.
- File uploads: validate MIME type, file extension, and magic bytes. Scan for image bombs (decompression attacks) by checking dimensions before processing.

### 7.2 Dependency Audit

- Run `npm audit` and address all high/critical vulnerabilities before release
- C++ dependencies: pin specific versions in CMake FetchContent. Review changelogs on update.
- Docker images: use specific version tags, not `latest`. Run `trivy` scan on all container images.

### 7.3 Secrets Management

- All secrets in environment variables, never in code
- `.env` files in `.gitignore`
- Production: use Docker secrets or a vault system
- JWT secrets: minimum 256 bits, rotated quarterly
- Database passwords: minimum 32 characters, randomly generated

---

## 8. Accessibility

### 8.1 Requirements

- WCAG 2.1 AA compliance minimum
- All interactive elements keyboard-navigable
- Screen reader support: semantic HTML, ARIA labels on custom components
- Color contrast ratio ≥ 4.5:1 for text, ≥ 3:1 for large text
- Focus indicators visible on all interactive elements
- Touch targets ≥ 44×44px (already required from Phase 1)
- Reduced motion: respect `prefers-reduced-motion` media query — disable animations

### 8.2 Testing

- Test with VoiceOver (iOS), TalkBack (Android), NVDA (desktop)
- axe DevTools browser extension: zero violations
- Lighthouse accessibility audit: ≥ 90

---

## 9. Monitoring & Observability

### 9.1 Logging

- Structured JSON logging: `{ "timestamp": "...", "level": "info", "method": "GET", "path": "/api/dashboard", "status": 200, "duration_ms": 45, "user_id": "..." }`
- Log levels: debug (dev only), info (requests, business events), warn (degraded behavior), error (failures)
- Log rotation: daily, 30-day retention
- Never log: passwords, tokens, full request bodies with PII

### 9.2 Health Checks

```
GET /api/health          → { "status": "ok", "version": "1.0.0", "uptime_seconds": 12345 }
GET /api/health/detailed → { "status": "ok", "db": "ok", "minio": "ok", "onnx": "ok", "ws_connections": 42 }
```

- Docker healthcheck: `curl -f http://localhost:8080/api/health || exit 1`
- Alert on: health check failure, error rate >1%, p95 latency >500ms, memory >500MB

### 9.3 Metrics (Future)

- Request rate, error rate, latency percentiles per endpoint
- WebSocket connection count
- Queue depth (inference tasks, sync operations)
- DB connection pool utilization
- Expose via Prometheus endpoint `/metrics` (use prometheus-cpp library)

---

## 10. Phase 4 Definition of Done

- [ ] FORGE is installable as a PWA on iOS and Android
- [ ] Workout logging works fully offline and syncs on reconnect
- [ ] Recent foods and routines are available offline
- [ ] Lighthouse scores: Performance ≥90, Accessibility ≥90, PWA ✅
- [ ] API response times are within budget under load (100 concurrent users)
- [ ] Rate limiting prevents abuse on all sensitive endpoints
- [ ] Users can export all data as CSV or JSON
- [ ] Users can import data from Strong, Hevy, and MyFitnessPal
- [ ] First-time user completes onboarding and logs first workout within 3 minutes
- [ ] All empty states have helpful content and CTAs
- [ ] Zero high/critical security vulnerabilities in dependency audit
- [ ] Health check endpoint is functional and monitored
- [ ] All forms and interactive elements are keyboard-accessible
- [ ] Structured logging is in place for all API requests

**The test scenario: You're in a gym basement with zero signal. You start your workout, log 5 exercises with sets/reps/weight — everything saves locally. You walk outside, phone reconnects, data syncs in <5 seconds. You open FORGE from your home screen (installed as PWA), it loads in under 1 second. You export your last 6 months of data as CSV for your coach. A new user downloads the app, completes onboarding in 45 seconds, starts their first workout from a suggested beginner routine, and logs their first meal — all within 3 minutes. Phase 4 is done.**

---

# 8. Project Structure

```
forge/
├── backend/
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── src/
│   │   ├── main.cpp                    # Server entry point
│   │   ├── config/
│   │   │   └── config.hpp              # Environment variable loading
│   │   ├── middleware/
│   │   │   ├── auth.hpp                # JWT validation middleware
│   │   │   ├── cors.hpp                # CORS middleware
│   │   │   ├── logging.hpp             # Request logging
│   │   │   └── rate_limit.hpp          # Rate limiting
│   │   ├── routes/
│   │   │   ├── auth_routes.cpp         # /api/auth/*
│   │   │   ├── workout_routes.cpp      # /api/workouts/*
│   │   │   ├── nutrition_routes.cpp    # /api/nutrition/*
│   │   │   ├── profile_routes.cpp      # /api/users/*
│   │   │   ├── analytics_routes.cpp    # /api/analytics/*
│   │   │   ├── social_routes.cpp       # /api/social/*
│   │   │   ├── feed_routes.cpp         # /api/feed/*
│   │   │   ├── program_routes.cpp      # /api/programs/*
│   │   │   ├── messaging_routes.cpp    # /api/conversations/*
│   │   │   ├── club_routes.cpp         # /api/clubs/*
│   │   │   └── coaching_routes.cpp     # /api/coaching/*
│   │   ├── services/
│   │   │   ├── auth_service.cpp        # Business logic: registration, login, tokens
│   │   │   ├── workout_service.cpp     # Workout CRUD, PR detection
│   │   │   ├── nutrition_service.cpp   # Food logging, USDA integration
│   │   │   ├── recognition_service.cpp # AI food recognition pipeline
│   │   │   ├── weight_service.cpp      # Weight tracking, EWMA trend
│   │   │   ├── coaching_service.cpp    # Adaptive TDEE, weekly check-in
│   │   │   ├── analytics_service.cpp   # Charts, progression, distribution
│   │   │   ├── social_service.cpp      # Follow/unfollow, reactions, comments
│   │   │   ├── feed_service.cpp        # Feed generation, visibility
│   │   │   ├── program_service.cpp     # Marketplace CRUD, upvotes, browse
│   │   │   ├── messaging_service.cpp   # Conversations, messages
│   │   │   ├── notification_service.cpp# Notification creation, delivery
│   │   │   ├── club_service.cpp        # Club CRUD, membership
│   │   │   └── export_service.cpp      # Data export (CSV, JSON)
│   │   ├── models/
│   │   │   ├── user.hpp
│   │   │   ├── workout.hpp
│   │   │   ├── exercise.hpp
│   │   │   ├── exercise_set.hpp
│   │   │   ├── nutrition_log.hpp
│   │   │   ├── food.hpp
│   │   │   ├── weight_entry.hpp
│   │   │   ├── program.hpp
│   │   │   ├── follow.hpp
│   │   │   ├── reaction.hpp
│   │   │   ├── comment.hpp
│   │   │   ├── notification.hpp
│   │   │   ├── conversation.hpp
│   │   │   ├── message.hpp
│   │   │   ├── club.hpp
│   │   │   └── badge.hpp
│   │   ├── db/
│   │   │   ├── pool.hpp                # Connection pool wrapper
│   │   │   └── migrations.hpp          # Migration runner
│   │   ├── ml/
│   │   │   ├── preprocessor.cpp        # Image preprocessing pipeline
│   │   │   ├── inference.cpp           # ONNX Runtime session management
│   │   │   └── food_classifier.cpp     # Food-specific classification logic
│   │   ├── ws/
│   │   │   ├── websocket_manager.hpp   # Connection tracking, channel subscriptions
│   │   │   └── websocket_handler.cpp   # Message routing
│   │   └── utils/
│   │       ├── jwt.hpp                 # JWT creation/verification
│   │       ├── bcrypt.hpp              # Password hashing
│   │       ├── uuid.hpp                # UUID generation
│   │       └── validation.hpp          # Input validation helpers
│   ├── tests/
│   │   ├── auth_test.cpp
│   │   ├── workout_test.cpp
│   │   ├── nutrition_test.cpp
│   │   ├── recognition_test.cpp
│   │   ├── weight_trend_test.cpp
│   │   ├── coaching_test.cpp
│   │   ├── social_test.cpp
│   │   ├── feed_test.cpp
│   │   └── integration/
│   │       └── api_test.cpp            # Full HTTP request/response tests
│   └── migrations/
│       ├── 001_create_users.sql
│       ├── 002_create_workouts.sql
│       ├── 003_create_exercises.sql
│       ├── 004_create_exercise_sets.sql
│       ├── 005_create_nutrition_log.sql
│       ├── 006_create_routines.sql
│       ├── 007_create_weight_entries.sql
│       ├── 008_create_progress_photos.sql
│       ├── 009_create_badges.sql
│       ├── 010_create_follows.sql
│       ├── 011_create_reactions.sql
│       ├── 012_create_comments.sql
│       ├── 013_create_notifications.sql
│       ├── 014_create_programs.sql
│       ├── 015_create_conversations.sql
│       ├── 016_create_messages.sql
│       └── 017_create_clubs.sql
├── frontend/
│   ├── package.json
│   ├── vite.config.ts
│   ├── tailwind.config.ts
│   ├── tsconfig.json
│   ├── index.html
│   ├── public/
│   │   ├── manifest.json
│   │   ├── service-worker.js
│   │   └── icons/
│   └── src/
│       ├── main.tsx
│       ├── App.tsx
│       ├── routes/
│       ├── components/
│       │   ├── ui/                     # Reusable primitives (Button, Input, Card, Modal)
│       │   ├── workout/               # Workout-specific components
│       │   ├── nutrition/             # Nutrition-specific components
│       │   ├── social/                # Feed, reactions, comments
│       │   ├── analytics/             # Charts, progress visualizations
│       │   └── layout/                # Navigation, sidebar, headers
│       ├── hooks/
│       ├── stores/                     # Zustand stores
│       ├── api/                        # TanStack Query hooks + API client
│       ├── utils/
│       └── styles/
├── docker-compose.yml
├── .env.example
├── Makefile
├── README.md
└── LICENSE                             # MIT
```

---

# 9. Design System

## Color Palette

| Token | Hex | Usage |
|-------|-----|-------|
| `bg-primary` | `#0A0A0F` | Main background |
| `bg-surface` | `#14141F` | Cards, panels |
| `bg-elevated` | `#1E1E2E` | Modals, dropdowns |
| `accent-primary` | `#6C5CE7` | CTAs, progress rings, active states |
| `accent-success` | `#00D68F` | Completed sets, under-target |
| `accent-warning` | `#FFB800` | Approaching target |
| `accent-danger` | `#FF5252` | Over target, errors |
| `accent-gold` | `#FFD700` | PRs, achievements |
| `text-primary` | `#F0F0F0` | Main text |
| `text-secondary` | `#8888A0` | Labels, descriptions |
| `text-muted` | `#555570` | Placeholders, disabled |

## Typography

- **Headings**: Inter Bold — H1: 28px, H2: 22px, H3: 18px
- **Body**: Inter Regular 16px, line-height 1.5
- **Data/Numbers**: Inter Tabular Nums (monospace alignment for columns)
- **Code**: JetBrains Mono 14px

## Spacing

4px base unit. Common values: 4, 8, 12, 16, 20, 24, 32, 48, 64px.

## Border Radius

Cards: 8px. Modals: 12px. Buttons: 24px. Pills/badges: 999px.

## Animation Timing

- Page transitions: 200ms ease-out
- Progress rings: 800ms ease-out
- PR celebration: 400ms scale bounce (0→1.2→1.0) + gold particle burst
- Set completion: 150ms scale pulse
- Number counter: 200ms

---

# 10. Success Metrics

| Metric | Target (6 months post-launch) | Why |
|--------|-------------------------------|-----|
| Workout sessions/week per active user | ≥ 3 | Proves logger is good enough for daily use |
| Food logs/day per active user | ≥ 2 meals | Proves nutrition tracking is fast enough |
| 30-day retention | ≥ 40% | Industry avg ~25%. Strava ~60%. Social drives retention. |
| Community programs published | ≥ 500 | Critical mass for marketplace |
| Average session duration | 3–8 min | Too short = not engaging. Too long = UX friction. |
| AI food recognition usage | ≥ 30% of food logs | Proves the AI feature adds value |
| WebSocket uptime | ≥ 99.5% | Real-time features must be reliable |

---

# Open Source Strategy

FORGE is MIT licensed. This is strategic:

1. **Trust** — users verify data isn't misused
2. **Contributions** — community submits exercises, fixes, translations
3. **Portfolio value** — open-source platform with stars >>> private repo
4. **Moat is community, not code** — open-sourcing code doesn't give away the moat

---

**This document is the complete specification. When building any feature, reference this document. When making design decisions, ask: "What does the best app in this category do? How does FORGE match or beat it?"**

**Build order: Phase 1 → Phase 2 → Phase 3 → Phase 4. Each phase should be fully functional before starting the next.**