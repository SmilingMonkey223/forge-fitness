# FORGE Phase 2 Implementation Guide

This document tracks the implementation status of Phase 2 features: AI & Intelligence.

## Overview

Phase 2 introduces:
- AI-powered food recognition (hybrid ONNX + LLM pipeline)
- Body weight tracking with EWMA trend smoothing
- Adaptive TDEE calculation from real user data
- Weekly coaching check-ins
- Comprehensive workout and nutrition analytics
- Progress photos with comparison
- Achievement badges and streaks

## Implementation Status

### ✅ Completed

#### Infrastructure
- [x] CMakeLists.txt updated with Phase 2 dependencies (CURL, OpenCV, ONNX Runtime, stb_image)
- [x] Database migrations created (008-012)
  - [x] Weight tracking table
  - [x] Progress photos table
  - [x] Weekly check-ins table
  - [x] Badges and streaks tables
  - [x] AI food recognition fields
- [x] Thread pool implementation for async ML inference
- [x] Header files for all major Phase 2 components

#### AI Food Recognition
- [x] Image preprocessing pipeline header (image_processor.hpp)
- [x] Image preprocessing implementation (stb_image-based)
  - [x] Image decoding (JPEG, PNG, WebP)
  - [x] Bilinear resizing
  - [x] ImageNet normalization
  - [x] NCHW tensor conversion
- [x] Food classifier header (ONNX Runtime integration)
- [x] LLM client header (Claude/GPT-4V integration)

#### Weight Tracking & Analytics
- [x] Weight tracker header with EWMA trend analysis
- [x] Weight tracker implementation
  - [x] CRUD operations for weight entries
  - [x] EWMA trend calculation (α = 0.1)
  - [x] Linear interpolation for missing days
  - [x] Rate of change calculation (weekly/monthly)
  - [x] Goal progress tracking
- [x] TDEE calculator header
- [x] Analytics service header

#### Gamification
- [x] Gamification service header (badges and streaks)
- [x] Badge definitions seeded in migration

### 🚧 In Progress

#### AI Food Recognition
- [ ] ONNX Runtime integration implementation
  - [ ] Model loading and session management
  - [ ] Inference execution
  - [ ] Top-K prediction extraction
  - [ ] Thread-safe session handling
- [ ] LLM client implementation
  - [ ] Anthropic Claude API integration
  - [ ] OpenAI GPT-4V API integration
  - [ ] Prompt engineering for food analysis
  - [ ] Response parsing
  - [ ] Rate limiting
- [ ] Food recognition service orchestration
  - [ ] Tier 1 → Tier 2 decision logic
  - [ ] Async task queue integration
  - [ ] Result caching
- [ ] Food recognition API endpoints
  - [ ] POST /api/nutrition/recognize (upload & analyze)
  - [ ] GET /api/nutrition/recognize/:task_id (polling)
  - [ ] POST /api/nutrition/recognize/confirm (log results)

#### TDEE & Coaching
- [ ] TDEE calculator implementation
  - [ ] Mifflin-St Jeor formula
  - [ ] Adaptive TDEE from weight + intake data
  - [ ] Energy density calculations
  - [ ] Macro target calculations
- [ ] Weekly check-in service
  - [ ] Check-in generation logic
  - [ ] Target adjustment algorithms
  - [ ] User acceptance/override handling
- [ ] Coaching API endpoints
  - [ ] GET /api/coaching/status
  - [ ] GET /api/coaching/expenditure
  - [ ] POST /api/coaching/checkin
  - [ ] PUT /api/coaching/targets

#### Analytics
- [ ] Analytics service implementation
  - [ ] Exercise progression queries
  - [ ] Muscle group distribution
  - [ ] PR detection and tracking
  - [ ] Training consistency heatmap
  - [ ] Macro intake aggregation
  - [ ] Adherence calculations
- [ ] Analytics API endpoints
  - [ ] GET /api/analytics/exercise/:id
  - [ ] GET /api/analytics/muscle-distribution
  - [ ] GET /api/analytics/prs
  - [ ] GET /api/analytics/consistency
  - [ ] GET /api/analytics/nutrition

#### Progress Photos
- [ ] Progress photo storage service
  - [ ] MinIO/S3 integration
  - [ ] Image resize and optimization
  - [ ] Presigned URL generation
- [ ] Progress photos API endpoints
  - [ ] POST /api/progress-photos
  - [ ] GET /api/progress-photos
  - [ ] GET /api/progress-photos/:id
  - [ ] DELETE /api/progress-photos/:id

#### Gamification
- [ ] Gamification service implementation
  - [ ] Badge criteria checking
  - [ ] Badge award logic
  - [ ] Streak calculation
  - [ ] Streak freeze logic
- [ ] Gamification API endpoints
  - [ ] GET /api/badges
  - [ ] GET /api/badges/user
  - [ ] GET /api/streaks
  - [ ] POST /api/streaks/freeze

### 📋 Not Started

#### Frontend
- [ ] Food recognition UI
  - [ ] Camera capture component
  - [ ] Image upload flow
  - [ ] AI results editing interface
  - [ ] Confidence indicators
  - [ ] Manual search fallback
- [ ] Weight tracking UI
  - [ ] Weight logging form
  - [ ] Weight trend chart (raw + trend lines)
  - [ ] Goal progress display
  - [ ] Days to goal estimate
- [ ] Analytics dashboards
  - [ ] Training analytics tab
    - [ ] Exercise progression charts
    - [ ] Muscle distribution donut chart
    - [ ] Volume per muscle bar chart
    - [ ] PR history list
    - [ ] Consistency heatmap
  - [ ] Nutrition analytics tab
    - [ ] Macro intake stacked bar chart
    - [ ] Calorie adherence display
    - [ ] Protein hit rate
    - [ ] Energy balance chart
- [ ] Weekly check-in UI
  - [ ] Check-in notification
  - [ ] Progress summary display
  - [ ] Target adjustment review
  - [ ] Accept/skip/override actions
- [ ] Progress photos UI
  - [ ] Photo upload (3 angles)
  - [ ] Photo gallery grid
  - [ ] Comparison slider
  - [ ] Weight overlay
- [ ] Badges & streaks UI
  - [ ] Badge collection display
  - [ ] Badge unlock animation
  - [ ] Streak counters
  - [ ] Streak freeze button

#### Testing
- [ ] Image preprocessing tests
  - [ ] Decode various formats
  - [ ] Resize accuracy
  - [ ] Normalization correctness
  - [ ] Memory leak tests (ASan)
- [ ] ONNX inference tests
  - [ ] Model loading
  - [ ] Inference correctness
  - [ ] Performance benchmarks
  - [ ] Thread safety
- [ ] Weight trend tests
  - [ ] EWMA calculation accuracy
  - [ ] Interpolation correctness
  - [ ] Rate calculation
- [ ] TDEE calculation tests
  - [ ] Formula TDEE accuracy
  - [ ] Adaptive TDEE edge cases
  - [ ] Energy density calculations
- [ ] Analytics tests
  - [ ] Exercise progression queries
  - [ ] PR detection logic
  - [ ] Adherence calculations
- [ ] Integration tests
  - [ ] Full food recognition pipeline
  - [ ] Weekly check-in flow
  - [ ] Badge award triggers
- [ ] Load tests
  - [ ] 10 concurrent food recognitions
  - [ ] 100 concurrent analytics queries
  - [ ] 1000 sequential recognitions (memory leak check)

#### Performance Optimization
- [ ] Image preprocessing < 50ms p95
- [ ] ONNX inference < 100ms p95 on CPU
- [ ] Full Tier 1 pipeline < 500ms p95
- [ ] Trend weight computation < 10ms
- [ ] Analytics queries < 200ms p95
- [ ] Progress photo upload < 2s p95

#### Documentation
- [ ] API documentation for Phase 2 endpoints
- [ ] Food recognition model setup guide
- [ ] ONNX Runtime installation instructions
- [ ] MinIO configuration guide
- [ ] Phase 2 deployment guide

## Dependencies

### Backend C++ Libraries
- **CURL**: HTTP client for LLM API calls
- **OpenCV** (optional): Advanced image preprocessing (falls back to stb_image)
- **ONNX Runtime**: ML inference for food classification
- **stb_image**: Lightweight image decoding (included via FetchContent)

### System Requirements
- C++20 compiler
- CMake 3.20+
- PostgreSQL 14+
- MinIO or S3-compatible storage
- 4+ CPU cores (for thread pool)

### External Services
- **Anthropic API** (optional): Claude for Tier 2 food analysis
- **OpenAI API** (optional): GPT-4V for Tier 2 food analysis

## Installation

### 1. Install System Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev libopencv-dev

# macOS
brew install curl opencv

# Download ONNX Runtime
# https://github.com/microsoft/onnxruntime/releases
# Extract to /usr/local/ or set onnxruntime_DIR
```

### 2. Get Food Classification Model

```bash
# Download a pre-trained food classification model
# Option 1: MobileNetV3-Small on Food-101 (recommended for starting)
# Option 2: EfficientNet-Lite0 on Food-2K (better accuracy)

# Example: Export PyTorch model to ONNX
python scripts/export_food_model.py --model mobilenetv3 --dataset food101 --output models/food_classifier.onnx

# Place model in: backend/models/food_classifier.onnx
```

### 3. Run Database Migrations

```bash
cd backend
./scripts/run_migrations.sh
```

### 4. Configure Environment

```bash
# Add to .env
ANTHROPIC_API_KEY=your_api_key_here
OPENAI_API_KEY=your_api_key_here
MINIO_ENDPOINT=localhost:9000
MINIO_ACCESS_KEY=minioadmin
MINIO_SECRET_KEY=minioadmin
FOOD_MODEL_PATH=/app/models/food_classifier.onnx
```

### 5. Build and Run

```bash
cd backend
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./forge
```

## Architecture

### AI Food Recognition Pipeline

```
User uploads image
    ↓
HTTP handler (Crow)
    ↓
Enqueue to ThreadPool
    ↓
ImageProcessor.process()
    ├─ decode (stb_image)
    ├─ resize (bilinear)
    ├─ normalize (ImageNet)
    └─ convert to NCHW tensor
    ↓
FoodClassifier.classify()
    ├─ ONNX Runtime inference
    └─ top-5 predictions
    ↓
If confidence < 0.7 OR user requests detail:
    ↓
LLMClient.analyze_food()
    ├─ POST to Anthropic/OpenAI
    ├─ parse JSON response
    └─ map to USDA database
    ↓
Return FoodRecognitionTask ID
    ↓
User polls GET /api/nutrition/recognize/:task_id
    ↓
User edits results
    ↓
POST /api/nutrition/recognize/confirm
    ↓
Log to nutrition_log table
```

### Weight Trend Calculation

```
User logs weight daily
    ↓
Stored in weight_entries table
    ↓
GET /api/weight/trend
    ↓
WeightTracker.get_trend()
    ├─ Fetch last 90 days
    ├─ Interpolate missing days (linear)
    ├─ Apply EWMA (α = 0.1)
    │   trend[t] = 0.1 × weight[t] + 0.9 × trend[t-1]
    ├─ Calculate weekly rate
    └─ Calculate goal progress
    ↓
Return trend data + rate + estimate
```

### Adaptive TDEE Calculation

```
Every Monday OR user requests check-in
    ↓
TDEECalculator.calculate_adaptive_tdee()
    ├─ Get last 14 days nutrition logs
    ├─ Get trend weights (14 days ago vs today)
    ├─ average_intake = Σ(calories) / 14
    ├─ weight_change = trend[today] - trend[14 days ago]
    ├─ energy_density = f(weight_change direction)
    │   └─ Loss: 0.7×7700 + 0.3×1800 = 5930 kcal/kg
    │   └─ Gain: 0.4×7700 + 0.6×1800 = 4160 kcal/kg
    ├─ deficit_or_surplus = weight_change × energy_density / 14
    └─ adaptive_tdee = average_intake - deficit_or_surplus
    ↓
Generate check-in recommendation
    ├─ Compare actual vs target rate
    ├─ Adjust calorie target ±50-100 kcal
    └─ Recalculate macros
    ↓
User accepts/overrides
    ↓
Update user profile targets
```

## Performance Targets

| Operation | Target | Measurement Point |
|-----------|--------|------------------|
| Image preprocessing | < 50ms p95 | Server-side timer |
| ONNX inference | < 100ms p95 | Server-side timer |
| Full Tier 1 recognition | < 500ms p95 | End-to-end |
| Full Tier 2 recognition | < 3000ms p95 | End-to-end (LLM latency) |
| Weight trend calculation | < 10ms | Server-side timer |
| Exercise progression query | < 200ms p95 | Server-side timer |
| Progress photo upload | < 2000ms p95 | Including resize + storage |

## Next Steps

1. **Complete ONNX Runtime integration** - Critical for Tier 1 food recognition
2. **Implement LLM client** - Claude API integration for Tier 2
3. **Build food recognition API endpoints** - Wire up the full pipeline
4. **Complete TDEE calculator** - Adaptive TDEE from real data
5. **Implement analytics service** - Queries for all charts
6. **Build frontend components** - UI for all Phase 2 features
7. **Comprehensive testing** - Unit, integration, load tests
8. **Performance optimization** - Meet all p95 targets

## Definition of Done

Phase 2 is complete when all items in the specification are checked off. See [forge phase2.md](forge%20phase2.md) for the full checklist.

**Test Scenario**: User eats lunch, snaps photo, app identifies "chicken breast, rice, broccoli" in < 3s. User adjusts rice portion, taps Log. Analytics page shows weight trend down 0.4 kg this week, TDEE at 2,580 kcal, bench press 1RM up 5 kg. Progress photos comparison shows visible change. "Macro Master" badge earned yesterday.
