# FORGE — Phase 2: AI & Intelligence

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