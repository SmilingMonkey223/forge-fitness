# FORGE - Product Requirements Document (Exact Feature Replica)

> **Strategy**: Copy exactly how Jeff Nippard's MacroFactor Workouts and Strava work, with custom FORGE UI
> **Version**: 2.0
> **Last Updated**: 2026-02-19

---

## Core Philosophy

**For Fitness Features**: Replicate MacroFactor Workouts EXACTLY
- Same workout tracking flow
- Same exercise library structure
- Same AI coaching algorithm logic
- Same progress visualization
- Same program structure

**For Social Features**: Replicate Strava EXACTLY
- Same feed structure
- Same kudos system
- Same clubs/challenges
- Same messaging
- Same privacy controls

**FORGE Difference**: Our own beautiful UI/UX, C++ backend, React frontend

---

## PART 1: WORKOUT FEATURES (MacroFactor Workouts Replica)

### 1.1 Workout Tracking - EXACT REPLICA

#### Logging Details (Copy Exactly)
```
What to track per set:
✓ Weight lifted (lbs/kg)
✓ Reps completed
✓ RIR (Reps in Reserve) - how many reps left in tank before failure
✓ Partial reps checkbox
✓ Rest time (auto-tracked)
✓ Unilateral tracking (left/right separately for single-arm/leg exercises)
✓ Drop sets
✓ Supersets (pair exercises)
✓ Failure sets (checkbox for "taken to failure")
✓ Exercise notes
```

#### Smart Features (Copy Exactly)
```
✓ Plate Calculator
  - Input: Available plates in gym (45, 35, 25, 10, 5, 2.5 lbs)
  - Input: Bar weight (45 lbs standard, 35 lbs women's, etc.)
  - Output: Exact plate combination per side to hit target weight

✓ Smart Warm-up Planner
  - Based on working weight, auto-generate 2-3 warm-up sets
  - Example: Working weight 225 lbs
    - Warm-up 1: 135 lbs × 5 (60%)
    - Warm-up 2: 185 lbs × 3 (82%)
    - Working sets: 225 lbs × target reps
```

#### UI Flow (Copy MacroFactor)
```
1. Select workout from program
2. View exercise list with:
   - Exercise name
   - Target sets × reps
   - RIR target (e.g., "3 RIR" = 3 reps left in tank)
   - Last week's performance
3. Tap exercise → Open set logging screen
4. Log set:
   - Weight: [____] lbs  (quick +/- buttons)
   - Reps: [____]
   - RIR: [____] (0-10 scale)
   - ☐ Partial reps
   - ☐ Taken to failure
5. Tap "Log Set" → Auto-start rest timer
6. Rest timer: Countdown with option to skip or add 30s
7. Repeat for all sets
8. Complete exercise → Next exercise
```

---

### 1.2 Exercise Library - EXACT REPLICA

#### Database Structure (638 Exercises)
```
Each exercise must have:
✓ Exercise name (e.g., "Barbell Bench Press")
✓ 3 camera angle videos (front, side, alternative)
✓ Detailed technique notes (written)
✓ Setup instructions
✓ Common mistakes list
✓ Primary muscle groups
✓ Secondary muscle groups
✓ Equipment required
✓ Exercise type: compound/isolation
✓ Laterality: bilateral/unilateral
```

#### Video Specifications
```
Format: MP4, embedded directly (no YouTube links)
Duration: 30-60 seconds per angle
Toggle: User can switch between 3 angles while watching
Accessible: During workout AND in exercise browser
No ads, no external clicks
```

#### Custom Exercise Creation
```
User can create custom exercises with:
1. Exercise name (text input)
2. Trackable metrics (checkboxes):
   ☐ Weight
   ☐ Reps
   ☐ Time
   ☐ Distance
3. Exercise type: Compound / Isolation
4. Laterality: Bilateral / Unilateral / N/A
5. Target muscle groups (multi-select):
   - Chest, Back, Shoulders, Arms, Legs, Core, etc.
6. Optional: Upload custom video
```

#### Exercise Substitution System
```
When user wants to swap exercise:
1. Tap "Replace Exercise" during workout
2. App suggests alternatives based on:
   - Same primary muscle group
   - Same equipment availability
   - Same exercise type (compound/isolation)
3. Examples:
   - Barbell Bench Press → Dumbbell Bench Press
   - Barbell Squat → Leg Press
   - Pull-ups → Lat Pulldown
4. Easy one-tap replacement
```

---

### 1.3 Program Structure - EXACT REPLICA

#### Two Modes (Copy MacroFactor)

**Mode 1: AI Coaching**
```
Onboarding questionnaire:
1. Primary goal:
   ○ Strength (1-5 rep focus)
   ○ Hypertrophy (6-12 rep focus)
   ○ Mixed (strength + size)
   ○ Athletic performance

2. Experience level:
   ○ Beginner (< 6 months training)
   ○ Intermediate (6 months - 2 years)
   ○ Advanced (2+ years)

3. Available equipment:
   ☐ Barbell
   ☐ Dumbbells
   ☐ Cables
   ☐ Machines
   ☐ Bodyweight only

4. Training frequency:
   ○ 2 days/week
   ○ 3 days/week
   ○ 4 days/week
   ○ 5 days/week
   ○ 6 days/week

AI generates program:
- Selects exercises from library matching equipment
- Creates periodization structure
- Sets rep ranges based on goal
- Assigns RIR targets
- Auto-updates weekly based on performance
```

**Mode 2: Tracker Mode**
```
Options:
1. Import pre-built FORGE programs (6 programs, copy Nippard's):
   - Min-Max Program
   - Bodybuilding Transformation System
   - Pure Bodybuilding Phase 1 & 2
   - Ultimate PPL System
   - Essentials Program

2. Create your own program:
   - Name program
   - Add workouts (e.g., "Push Day", "Pull Day")
   - Add exercises per workout
   - Set target sets/reps/RIR
   - Save and run

3. Track any program:
   - Just log workouts manually without following structure
```

#### Periodization Logic (Copy MacroFactor)
```
Main compound lifts (Squat, Bench, Deadlift, OHP):
- Undulated periodization
- Week 1: 4×6 @ 7 RIR
- Week 2: 3×8 @ 6 RIR
- Week 3: 5×5 @ 8 RIR
- Week 4: 4×6 @ 7 RIR
- Pattern: Sets, reps, intensity vary week-to-week

Accessory/isolation exercises:
- Linear periodization
- Week 1-4: Same sets/reps, increase weight
- Example: 3×12 @ 3 RIR, increase 5 lbs each week

Deload weeks:
- Every 4-6 weeks
- Reduce volume by 40%
- Maintain intensity (RIR stays same)
```

---

### 1.4 AI Coaching Algorithm - EXACT REPLICA

#### Rule-Based Logic (NOT Generative AI)
```
MacroFactor explicitly says: "Research-backed algorithms, not black-box AI"

Our implementation:
1. Performance tracking per exercise
2. Fatigue detection based on RIR trends
3. Progression rules based on exercise science
4. Weekly check-in adjustments
```

#### Progression Algorithm
```python
# Example progression logic (simplified)

def calculate_next_week_weight(exercise_history):
    last_week_sets = get_last_week_sets(exercise_history)

    # Rule 1: If all sets hit target RIR or better
    if all(set.rir >= target_rir for set in last_week_sets):
        return last_week_weight + 5  # lbs (2.5 kg for smaller muscles)

    # Rule 2: If most sets were 1-2 RIR below target (too hard)
    elif avg_rir(last_week_sets) < target_rir - 2:
        return last_week_weight - 5  # Reduce weight, recover

    # Rule 3: If RIR too high (too easy)
    elif avg_rir(last_week_sets) > target_rir + 2:
        return last_week_weight + 10  # Jump up faster

    # Rule 4: Maintain weight if close
    else:
        return last_week_weight

def calculate_next_week_reps(exercise_history):
    # Similar logic for rep adjustments within rep range
    # If hitting top of rep range with good RIR → increase weight
    # If hitting bottom with poor RIR → increase reps or reduce weight
```

#### Learning From Performance
```
Data tracked per exercise:
- Weekly volume (sets × reps × weight)
- RIR trend (are you consistently hitting targets?)
- Strength progression rate (weight increased per week)
- Fatigue indicators (RIR declining week-over-week = fatigue)

Algorithm adjusts:
- If fatigue detected → insert deload or reduce volume
- If progression stalled → change rep range or exercise variation
- If crushing targets → accelerate progression
```

#### Weekly Check-In
```
Every Sunday:
1. User rates week:
   - Hunger: 1-5 (😫 to 😊)
   - Energy: 1-5
   - Recovery: 1-5
   - Training intensity: 1-5

2. App reviews performance:
   - Workouts completed: 4/4 ✓
   - Volume change: +8% vs last week
   - PRs achieved: 2
   - RIR adherence: 92%

3. App makes adjustments:
   - If low energy + high volume → reduce volume 10%
   - If crushing workouts → increase volume or intensity
   - If missing workouts → suggest schedule changes
```

---

### 1.5 Dashboard & Progress Visualization - EXACT REPLICA

#### Customizable Dashboard (Copy MacroFactor)
```
User can show/hide and reorder sections:

Available widgets:
☐ Volume Tracker (per muscle group)
☐ PR Tracker (recent personal records)
☐ Progress Charts (weight progression per exercise)
☐ Muscle Group Breakdown (body map visualization)
☐ Achievements (badges, streaks)
☐ Workout History (calendar view)
☐ Body Metrics (weight, progress photos)
☐ Upcoming Workouts (from program)
```

#### Volume Tracker
```
Chart type: Bar graph
X-axis: Weeks (last 4-12 weeks)
Y-axis: Total volume (lbs lifted)
Data series:
- Chest volume
- Back volume
- Legs volume
- Shoulders volume
- Arms volume

Visualization:
Week 1: Chest 18,500 | Back 15,200 | Legs 22,000
Week 2: Chest 19,200 | Back 16,100 | Legs 23,500
Week 3: Chest 20,100 | Back 16,800 | Legs 24,200
Week 4: Chest 21,000 | Back 17,500 | Legs 25,000

Color-coded bars per muscle group
```

#### PR Tracker
```
List of recent PRs:
• Bench Press: 225 lbs × 5 (Feb 19) ← New!
• Squat: 315 lbs × 3 (Feb 15)
• Deadlift: 405 lbs × 1 (Feb 10)
• Overhead Press: 145 lbs × 8 (Feb 19) ← New!

Highlight: Gold badge for new PRs this week
Historical: Click exercise to see PR progression over time
```

#### Progress Charts (Per Exercise)
```
Chart: Line graph
Example: Bench Press progression

Data points: Estimated 1RM over time
Calculation: Brzycki formula
  1RM = weight × (36 / (37 - reps))

Visual:
Jan 1:  215 lbs (from 185×8)
Jan 15: 220 lbs (from 195×7)
Feb 1:  223 lbs (from 205×6)
Feb 19: 235 lbs (from 225×5) ← PR!

Trend line: +20 lbs in 7 weeks
```

#### Muscle Group Body Map
```
Interactive visualization:
- 3D body model or flat diagram
- Color intensity = volume per muscle

Example:
Chest: 21,000 lbs (this week) → Dark blue
Back: 17,500 lbs → Medium blue
Shoulders: 9,500 lbs → Light blue
Legs: 25,000 lbs → Darkest blue
Arms: 8,200 lbs → Lightest blue

User can:
- Tap muscle group → See exercise breakdown
- View weekly, monthly, or 3-month data
```

#### Body Metrics Tracker
```
Weight tracking:
- Daily weigh-ins
- EWMA trend line (α = 0.1, smooth fluctuations)
- Chart with dots (daily) + trend line

Progress photos:
- Upload front/side/back photos
- Tag with date
- View timeline with slider
- Side-by-side comparison mode

Measurements (optional):
- Chest, waist, arms, thighs circumference
- Track over time
```

---

### 1.6 Workout Logging Workflow - EXACT UI FLOW

#### Pre-Workout Screen
```
┌─────────────────────────────────┐
│  FORGE  💪  Today: Mon, Feb 19  │
├─────────────────────────────────┤
│                                 │
│  📅 Scheduled Workout           │
│  ┌───────────────────────────┐ │
│  │ Push Day A                │ │
│  │ Chest, Shoulders, Triceps │ │
│  │ 5 exercises • ~60 min     │ │
│  │                           │ │
│  │ [Start Workout]           │ │
│  └───────────────────────────┘ │
│                                 │
│  OR                             │
│                                 │
│  [Quick Start Empty Workout]    │
│  [Browse Programs]              │
│                                 │
│  ────────────────────────────   │
│  Recent Workouts:               │
│  • Push Day A - 3 days ago      │
│  • Pull Day A - 5 days ago      │
│  • Leg Day - 7 days ago         │
└─────────────────────────────────┘
```

#### During Workout - Exercise List
```
┌─────────────────────────────────┐
│ ← Push Day A       ⏱️ 12:34     │
│                      [Finish]   │
├─────────────────────────────────┤
│                                 │
│ ✅ Bench Press                  │
│    3 sets • 225 lbs × 5 PR! 🏆  │
│                                 │
│ ▶️  Incline Dumbbell Press      │
│    0/3 sets • Last: 70 lbs × 10 │
│    Target: 8-12 @ 3 RIR         │
│    [📹 View Form] [START SET]   │
│                                 │
│ ⚪ Overhead Press                │
│    0/4 sets • Last: 135 × 8     │
│    Target: 6-8 @ 2 RIR          │
│                                 │
│ ⚪ Lateral Raises                │
│    0/3 sets • Last: 25 × 15     │
│                                 │
│ ⚪ Tricep Pushdowns              │
│    0/3 sets • Last: 70 × 12     │
│                                 │
│ [+ Add Exercise]                │
│ [+ Add Notes]                   │
└─────────────────────────────────┘
```

#### Logging a Set (EXACT MacroFactor Flow)
```
┌─────────────────────────────────┐
│  Incline Dumbbell Press         │
│  Set 1 of 3                     │
├─────────────────────────────────┤
│                                 │
│  Weight (lbs)                   │
│  ┌──────────────────────────┐  │
│  │   [-10]  [-5]  [+5] [+10]│  │
│  │        [  70  ]          │  │
│  └──────────────────────────┘  │
│                                 │
│  Reps                           │
│  ┌──────────────────────────┐  │
│  │    [-1]        [+1]      │  │
│  │        [  10  ]          │  │
│  └──────────────────────────┘  │
│                                 │
│  RIR (Reps in Reserve)          │
│  ┌──────────────────────────┐  │
│  │ 0  1  2 [3] 4  5  6  7+  │  │
│  │     (3 selected)         │  │
│  └──────────────────────────┘  │
│                                 │
│  ☐ Partial reps                 │
│  ☐ Taken to failure             │
│                                 │
│  📊 Last set: 70 lbs × 10 @ 3RIR│
│  🎯 Target: 8-12 reps @ 3 RIR   │
│                                 │
│  [✓ Log Set]                    │
│  [Skip Set]                     │
└─────────────────────────────────┘
```

#### Rest Timer (Full Screen)
```
┌─────────────────────────────────┐
│                                 │
│          ⏱️ REST                 │
│                                 │
│           01:23                 │
│                                 │
│   ████████████████░░░░░░       │
│           30s left              │
│                                 │
│   [Skip Rest]    [Add 30s]      │
│                                 │
│   ────────────────────────────  │
│                                 │
│   Next: Set 2 of 3              │
│   Incline Dumbbell Press        │
│                                 │
└─────────────────────────────────┘
```

#### Exercise Options (Swipe Menu)
```
During workout, swipe left on exercise or tap "⋮":

┌─────────────────────────────────┐
│  Exercise Options               │
├─────────────────────────────────┤
│  📹 View Form Video             │
│  🔁 Replace Exercise            │
│  ➕ Add Warm-up Sets            │
│  ✏️ Edit Sets/Reps/RIR          │
│  🔗 Pair as Superset            │
│  📝 Add Exercise Notes          │
│  ⚙️ Exercise Settings           │
│  🗑️ Remove from Workout         │
└─────────────────────────────────┘
```

#### Post-Workout Summary
```
┌─────────────────────────────────┐
│  🎉 Workout Complete!           │
├─────────────────────────────────┤
│                                 │
│  ⏱️  Duration: 52 minutes        │
│  🏋️  Total Volume: 12,450 lbs   │
│  💪 Exercises: 5                 │
│  ✅ Sets: 15                     │
│                                 │
│  🏆 New PRs:                    │
│  • Bench Press: 225 × 5         │
│  • Overhead Press: 145 × 8      │
│                                 │
│  How did it feel?               │
│  😫  😐  💪  🔥                  │
│  Tap to rate intensity (1-4)    │
│                                 │
│  Notes (optional):              │
│  [________________________]     │
│                                 │
│  📸 Add Progress Photo           │
│                                 │
│  ☐ Share to Feed (friends only) │
│                                 │
│  [Done]                         │
└─────────────────────────────────┘
```

---

## PART 2: SOCIAL FEATURES (Strava Replica)

### 2.1 Activity Feed - EXACT STRAVA REPLICA

#### Feed Structure
```
Two ordering options (toggle at top):
1. Personalized (algorithm)
2. Latest Activities (chronological)

Algorithm rules (copy Strava):
- Activities you interact with appear more
- Athletes you give kudos to show up more
- Efforts you've "missed" (e.g., friend's PR when you were offline)
```

#### Feed Post Display
```
┌─────────────────────────────────┐
│  @alex_lifts  •  2h ago         │
│  ────────────────────────────   │
│  💪 Completed: Push Day A       │
│  52 min • 12,450 lbs volume     │
│  🏆 2 PRs                        │
│                                 │
│  [See workout details]          │
│                                 │
│  👍 24   💬 8   🔥 3             │
└─────────────────────────────────┘

Stats shown automatically (like Strava):
- Duration (if > 30 min)
- Volume (total lbs lifted)
- PR count (if any)
- Exercise count

Achievement banner (if applicable):
- "🏆 New PR!" for personal records
- "🔥 5-day streak!" for consistency
- "⭐ First time: Deadlifts"
```

#### Feed Interactions
```
Actions:
1. Tap post → View full workout details
2. Tap 👍 → Give kudos (permanent, can't undo)
3. Tap 💬 → View/add comments
4. Tap username → View profile
5. Long-press → Options:
   - Share
   - Report
   - Hide this post
   - Mute @username

Kudos limitations (copy Strava):
- Rate limited: Max kudos per hour
- If limit hit: Wait 1 hour
- Permanent action (can't undo)
```

---

### 2.2 Kudos System - EXACT STRAVA REPLICA

#### How It Works
```
Giving kudos:
1. Method 1: Tap 👍 on individual workout post
2. Method 2: "Kudos bomb" for group workouts
   - Tap "Manage Group" on group workout
   - Shake phone
   - Gives kudos to everyone in group

Kudos characteristics:
- Permanent (cannot undo, like a high-five)
- Shows who gave kudos (list of usernames)
- Count visible on post

Rate limiting (exact Strava rules):
- Maximum kudos per hour enforced
- If exceeded: "You've given too many kudos. Wait 1 hour."
- Persistent issues: "Wait 24 hours before giving more kudos."
```

#### UI Display
```
Post showing kudos:
┌─────────────────────────────────┐
│  @jordan_fit  •  4h ago         │
│  ────────────────────────────   │
│  🏆 New PR! Deadlift 405×1      │
│  [Progress photo]               │
│  "Finally hit 4 plates!"        │
│                                 │
│  👍 48   💬 15   🔥 12           │
│  Liked by @alex, @sam, and      │
│  46 others                      │
└─────────────────────────────────┘

Tap kudos count:
┌─────────────────────────────────┐
│  Kudos (48)                     │
│  ────────────────────────────   │
│  @alex_lifts                    │
│  @sam_strong                    │
│  @morgan_fit                    │
│  @chris_gains                   │
│  ... 44 more                    │
└─────────────────────────────────┘
```

---

### 2.3 Comments - EXACT STRAVA REPLICA

#### Comment System
```
Features:
✓ Comment on workouts, PRs, progress photos
✓ Manual @tagging (no inline threading)
✓ Automatic subscription to conversation
✓ Notification for new comments

Limitations (same as Strava):
✗ NO comment threading (flat list)
✗ NO inline replies (workaround: @mention)
✗ Cannot edit comments after posting
✗ Can only delete your own comment
```

#### UI Flow
```
Viewing comments:
┌─────────────────────────────────┐
│  💬 8 Comments                  │
│  ────────────────────────────   │
│                                 │
│  @jordan_fit  •  2h ago         │
│  "Beast mode! 💪"               │
│  👍 3                            │
│                                 │
│  @sam_strong  •  1h ago         │
│  "Nice PRs! How's your shoulder?"│
│  👍 1                            │
│                                 │
│  @alex_lifts  •  30m ago        │
│  "@sam_strong feeling great!"   │
│  👍 0                            │
│                                 │
│  You: [Type a comment...]       │
│                                 │
└─────────────────────────────────┘

Notifications:
- Push: "Jordan commented on your workout"
- In-app: Red dot on bell icon
- Email: (configurable)

Managing notifications:
Settings > Notifications > Comments
☐ Push notifications
☐ Email notifications
☐ In-app only

Unsubscribe from conversation:
- Only option: Delete your comment
```

---

### 2.4 Clubs - EXACT STRAVA REPLICA

#### Club Structure
```
Two club types:
1. Public clubs: Anyone can join
2. Invite-only clubs: Owner/admin must approve

Creating a club:
1. Tap "Create Club" on clubs page
2. Fill out:
   - Club name
   - Description
   - Sport type (Strength Training, Bodybuilding, Powerlifting, etc.)
   - Visibility: Public / Invite-only
   - Club photo
3. Create → You're the owner
```

#### Club Leaderboards (EXACT Strava Logic)
```
Ranking logic:
- Single-sport clubs: Ranked by total volume (lbs lifted)
- Multi-sport clubs: Ranked by total workout time

Reset schedule:
- Every Sunday at 11:59 PM
- Only current week shown
- Historical data NOT retained

Leaderboard display:
┌─────────────────────────────────┐
│  Morning Warriors Club          │
│  Leaderboard (This Week)        │
│  ────────────────────────────   │
│  1. @alex_lifts     45,200 lbs  │
│  2. @jordan_fit     42,800 lbs  │
│  3. @sam_strong     41,500 lbs  │
│  4. @morgan_gains   38,900 lbs  │
│  ...                            │
│  12. You            28,400 lbs  │
│  ────────────────────────────   │
│  Total club volume: 1,247,300   │
└─────────────────────────────────┘
```

#### Club Challenges (EXACT Strava)
```
IMPORTANT: Cannot create challenges within clubs

Workaround: Group Challenges (separate feature)
- Create Group Challenge
- Invite up to 24 athletes
- Private leaderboard (only participants see)
- Tracked separately from club

Subscription-based:
- Free users: Join 3 challenges max
- Subscribers: Unlimited challenges

Types:
1. Strava official challenges: Public leaderboards
2. Group challenges: Private leaderboards
```

#### Club Features
```
Club home page:
┌─────────────────────────────────┐
│ ← Morning Warriors              │
│   127 members                   │
│   [Join] / [Leave]              │
│                                 │
│   [📊 Leaderboard] [👥 Members] │
│   [💬 Feed]        [⚙️ Settings] │
│                                 │
│   ────────────────────────────  │
│   RECENT ACTIVITY               │
│                                 │
│   @alex_lifts completed Push Day│
│   2h ago • 12,450 lbs           │
│                                 │
│   @jordan_fit hit new PR        │
│   4h ago • Deadlift 405×1       │
│                                 │
│   [View Full Feed]              │
└─────────────────────────────────┘
```

---

### 2.5 Following/Followers & Privacy - EXACT STRAVA

#### Privacy Settings (Two Main Options)

**Option 1: "Everyone" (Default for 18+)**
```
Non-followers can view:
✓ Complete profile
✓ Recent workout stats
✓ Recent photos
✓ Calendar widget
✓ Clubs membership
✓ Recent achievements

Following behavior:
- Anyone can follow without approval
- Workouts appear in their feed
```

**Option 2: "Followers"**
```
Followers see:
✓ Complete profile details
✓ Photo, location, bio
✓ Recent photos, clubs, achievements
✓ Trophy case
✓ Following/followers lists
✓ Detailed workout info

Non-followers see:
✓ Profile photo
✓ Bio
✓ Count of following/followers (number only)
✓ Mutual follows

Following behavior:
- Approval required before follow
- Request sent → You approve/deny
```

#### Profile Discovery
```
Finding people:
1. Search by username
2. Suggested from contacts (opt-in)
3. Popular in your area
4. Similar goals/interests
5. Mutual friends

Follow flow:
1. Tap username → View profile
2. Tap "Follow"
3. If public: Instant follow
4. If private: Request sent, pending approval
```

#### Settings Location
```
Web: Profile icon (top right) > Settings > Privacy Controls

Mobile: Settings (gear icon) > Privacy Controls
  - Profile Page: Everyone / Followers
  - Activities: Everyone / Followers / Only You
  - Messages: Nobody / Following / Mutuals
```

---

### 2.6 Activity Privacy - EXACT STRAVA

#### Three Privacy Levels (Per Workout)

**Everyone (Public)**
```
✓ Visible to all FORGE users
✓ Appears on profile
✓ Appears in club feeds
✓ Appears on public leaderboards
✓ Full details visible
```

**Followers**
```
✓ Visible to followers only
✗ Does NOT appear on segment leaderboards
✗ Does NOT appear on challenge leaderboards
✓ Still shows on your profile (to followers)
✓ Segment matches visible on activity detail page (with times)
```

**Only You (Private)**
```
✓ Visible only to you
✗ Does NOT appear on profile or feeds
✗ Does NOT appear on any leaderboards
✗ Completely private
```

#### Default Setting
```
Settings > Privacy Controls > Activities
○ Everyone (default)
○ Followers
○ Only You

Applies to all future workouts.
Can change individual workout privacy after posting.
```

---

### 2.7 Messaging - EXACT STRAVA

#### Message Types

**Direct Messages (1-on-1)**
```
Features:
- Send text messages
- Share routes (workout templates)
- Share workout activities
- Real-time chat

Privacy controls:
Settings > Messaging
○ Nobody (disabled)
○ Following (people you follow can message)
○ Mutuals (only mutual follows can message)
```

**Group Messages**
```
Features:
- Up to 25 people per group
- Share routes and activities
- Group text chat
- Name the group

Creating:
1. Tap "New Message"
2. Select "New Group"
3. Add participants (up to 25)
4. Name group
5. Send first message
```

#### Sharing Workouts in Messages
```
In chat:
1. Tap route icon in text box
2. Select from:
   - Saved workout templates
   - Completed workouts
   - Create new workout
3. Sends as rich preview:
   ┌─────────────────────────┐
   │  Push Day A             │
   │  5 exercises • ~60 min  │
   │  [View Workout]         │
   └─────────────────────────┘
4. Recipient can tap to view/copy
```

#### Platform Availability
```
Mobile app: Full messaging features
Web app: Read-only (view messages, cannot send)
```

---

### 2.8 User Profile - EXACT STRAVA STRUCTURE

#### Profile Components

**Logged-In View (All Info)**
```
┌─────────────────────────────────┐
│  [Profile Photo]                │
│  @alex_lifts                    │
│  Alex Johnson                   │
│  📍 San Francisco, CA           │
│                                 │
│  "Building strength, one rep    │
│  at a time. 💪"                 │
│                                 │
│  127 followers • 98 following   │
│  [Edit Profile]                 │
│                                 │
│  ────────────────────────────   │
│  STATS                          │
│  This Month:                    │
│  🏋️ 16 workouts                 │
│  💪 342,500 lbs volume          │
│  ⏱️ 14.5 hours                  │
│                                 │
│  All Time:                      │
│  🏋️ 324 workouts                │
│  💪 6.8M lbs volume             │
│  ⏱️ 287 hours                   │
│                                 │
│  ────────────────────────────   │
│  🏆 ACHIEVEMENTS                │
│  • 100 Workouts                 │
│  • 30-Day Streak                │
│  • 1M lbs Lifted                │
│  [View All]                     │
│                                 │
│  ────────────────────────────   │
│  📅 ACTIVITY CALENDAR           │
│  [Last 4 weeks grid]            │
│  Mon Tue Wed Thu Fri Sat Sun    │
│  ■   ■   ■   ■   □   ■   □     │
│  ■   ■   ■   □   ■   ■   ■     │
│  ...                            │
│                                 │
│  ────────────────────────────   │
│  📊 TOP EXERCISES               │
│  1. Bench Press (52 sessions)  │
│  2. Squat (48 sessions)         │
│  3. Deadlift (40 sessions)      │
│                                 │
│  ────────────────────────────   │
│  🏘️ CLUBS                       │
│  • Morning Warriors (127)       │
│  • Powerlifting Club (543)      │
│                                 │
│  ────────────────────────────   │
│  RECENT WORKOUTS                │
│  [List of recent workouts]      │
└─────────────────────────────────┘
```

**Logged-Out View (Limited Info)**
```
If profile = "Everyone":
✓ Full name
✓ Recent workouts (titles only)
✓ Trophy case
✓ Current month stats (distance/time totals)
✓ Following/followers counts (numbers only)

If profile = "Followers":
✓ Profile photo
✓ Bio
✓ Following/followers counts
✓ Mutual follows
✗ Workouts hidden
✗ Stats hidden
✗ Achievements hidden
```

---

### 2.9 Segments & Competitions (Adapted for Fitness)

**Concept**: "Segments" in Strava are portions of routes. In FORGE, "segments" = **specific exercises**

#### Exercise Leaderboards
```
For each exercise (e.g., Bench Press):

Global leaderboards:
1. Heaviest single rep (1RM)
   - 1. @sam_strong: 500 lbs
   - 2. @alex_lifts: 450 lbs
   - 3. @jordan_fit: 425 lbs

2. Most volume in single session
   - 1. @morgan_gains: 18,500 lbs
   - 2. @chris_fit: 17,200 lbs

3. Most reps at bodyweight
   - 1. @alex_lifts: 25 reps
   - 2. @sam_strong: 22 reps

Filters:
- All-time
- This month
- This week
- Gender (Men's / Women's)
- Weight class
```

#### Achievements (Copy Strava)
```
Exercise-specific:
- CR (Course Record): Heaviest 1RM all-time
- KOM (King of the Mountain): Heaviest men's 1RM
- QOM (Queen of the Mountain): Heaviest women's 1RM
- Local Legend: Most sessions with this exercise in 90 days

Icons:
- 👑 Crown for 1st place on exercise leaderboard
- 🏆 Laurel wreath for Local Legend
```

#### Live Exercise Challenges
```
During workout:
- Real-time notification if close to PR
- "You're 5 lbs away from your Bench Press PR!"
- If PR achieved: Confetti animation + "New PR!" banner

Post-workout:
- "You ranked #12 globally for Bench Press this week"
- "You moved up 3 spots on the Deadlift leaderboard"
```

---

## PART 3: IMPLEMENTATION PRIORITY

### Phase 1: Workout Tracking Core (Weeks 1-4)
```
✓ Exercise library (638 exercises, 3-angle videos)
✓ Custom exercise creation
✓ Workout logging (weight, reps, RIR, rest timer)
✓ Smart warm-up planner
✓ Plate calculator
✓ Superset/drop set tracking
```

### Phase 2: Program Structure (Weeks 5-6)
```
✓ AI coaching mode (questionnaire + program generation)
✓ Tracker mode (import pre-built programs)
✓ Periodization logic
✓ Exercise substitution system
✓ Weekly check-ins
```

### Phase 3: Progress Visualization (Weeks 7-8)
```
✓ Customizable dashboard
✓ Volume tracker (per muscle group)
✓ PR tracker
✓ Progress charts (per exercise)
✓ Body map visualization
✓ Weight tracking + EWMA trend
✓ Progress photos
```

### Phase 4: AI Coaching Algorithm (Weeks 9-10)
```
✓ Rule-based progression logic
✓ Performance learning (RIR trends, fatigue detection)
✓ Automatic program adjustments
✓ Weekly summary generation
```

### Phase 5: Social Feed & Kudos (Weeks 11-12)
```
✓ Activity feed (personalized + latest)
✓ Workout posts
✓ Kudos system (with rate limiting)
✓ Comments (no threading, manual tagging)
✓ Notifications
```

### Phase 6: Clubs & Messaging (Weeks 13-14)
```
✓ Club creation (public/private)
✓ Club leaderboards (weekly reset)
✓ Group challenges (up to 24 people)
✓ Direct messaging (1-on-1)
✓ Group messaging (up to 25)
```

### Phase 7: Privacy & Profiles (Weeks 15-16)
```
✓ Privacy controls (Everyone/Followers/Only You)
✓ Activity privacy per workout
✓ Message privacy settings
✓ User profiles (full + limited views)
✓ Following/followers system
```

### Phase 8: Leaderboards & Achievements (Weeks 17-18)
```
✓ Exercise leaderboards (global, monthly, weekly)
✓ Achievements (KOM/QOM/CR/Local Legend)
✓ Live PR notifications
✓ Segment challenges
```

### Phase 9: Polish & Testing (Weeks 19-20)
```
✓ Performance optimization
✓ UI/UX refinement
✓ Bug fixes
✓ Load testing
✓ Security audit
```

---

## PART 4: TECHNICAL ARCHITECTURE

### Backend (C++20)
```
Framework: Crow HTTP
Database: PostgreSQL 16 (libpqxx)
Auth: JWT (HS256)
File storage: S3 (exercise videos, progress photos)
Real-time: WebSockets (for live notifications)
```

### Frontend (React 18 + TypeScript)
```
State: Zustand (global), TanStack Query (server)
Routing: React Router v6
Styling: Tailwind CSS + Headless UI
Charts: Recharts (volume, progress graphs)
Video: HTML5 video player with angle toggle
```

### Database Schema (Key Tables)
```
users
user_profiles
exercises (638 rows)
exercise_videos (638 exercises × 3 angles = 1,914 videos)
workouts
workout_exercises
exercise_sets (weight, reps, rir, partial, failure, rest_time)
programs
program_workouts
user_programs
weight_entries
progress_photos
clubs
club_members
club_leaderboards
messages (DMs)
group_messages
feed_posts
kudos
comments
exercise_leaderboards
achievements
```

---

## PART 5: KEY DIFFERENCES FROM CURRENT BUILD

### What We Already Have (Phase 1 Foundation)
```
✓ Authentication (JWT, PBKDF2)
✓ PostgreSQL database (12 tables)
✓ Basic React frontend
✓ Dashboard UI
✓ Docker setup
✓ CI/CD pipeline
```

### What Needs to Change
```
1. Exercise library:
   - Current: Seeded 200+ exercises
   - Needed: 638 exercises with 3-angle videos each

2. Workout logging:
   - Current: Basic sets/reps tracking
   - Needed: RIR, partial reps, failure, supersets, drop sets

3. Smart features:
   - Add: Plate calculator
   - Add: Smart warm-up planner
   - Add: Exercise substitution system

4. Program structure:
   - Add: AI coaching mode
   - Add: Periodization logic
   - Add: Weekly check-ins

5. Social features:
   - Add: Feed, kudos, comments
   - Add: Clubs, challenges
   - Add: Messaging
   - Add: Privacy controls

6. Progress tracking:
   - Expand: Volume per muscle group
   - Add: Body map visualization
   - Expand: Exercise leaderboards
```

---

## PART 6: OPEN QUESTIONS

### Video Hosting
```
Q: Where to host 1,914 exercise videos (638 × 3 angles)?
Options:
1. Self-host on S3 (AWS/Cloudflare R2)
   - Pros: Full control, no API limits
   - Cons: Bandwidth costs
2. YouTube unlisted embeds
   - Pros: Free hosting
   - Cons: Ads, slow loading
3. Vimeo private videos
   - Pros: No ads, good quality
   - Cons: Monthly fee

Decision needed: Budget vs user experience
```

### Exercise Video Creation
```
Q: Who creates 638 exercises × 3 angles?
Options:
1. Film yourself (638 exercises!)
2. License from existing library (e.g., ExRx.net)
3. Hire videographer
4. Start with 100 most common exercises, expand later

Decision needed: Time vs cost vs scope
```

### AI Coaching Backend
```
Q: How to implement rule-based progression algorithm?
Options:
1. Hardcoded rules in C++ (fastest, inflexible)
2. JSON-based rule engine (flexible, maintainable)
3. Hybrid: Core logic in C++, rules in database

Decision needed: Flexibility vs performance
```

### Messaging Infrastructure
```
Q: Real-time messaging implementation?
Options:
1. WebSockets (Crow supports, need to implement)
2. Polling (simple, inefficient)
3. Third-party (Firebase, Pusher) - add dependency

Decision needed: Complexity vs features
```

---

## SUCCESS METRICS

### Engagement (Copy Strava Metrics)
```
- Daily Active Users (DAU)
- Workouts logged per user per week
- Kudos given per user per week
- Comments per user per week
- Club participation rate
```

### Retention
```
- D1, D7, D30 retention (same as Strava tracks)
- Workout adherence vs program (% completed)
- Weekly active users (WAU)
```

### Quality
```
- App load time < 2s
- API latency < 200ms p95
- Video streaming < 3s to first frame
- Crash-free sessions > 99.5%
```

---

## NEXT STEPS

1. **User Approves This PRD**
   - Confirm exact features match MacroFactor + Strava
   - Answer open questions (videos, messaging, AI logic)

2. **Create Implementation Plan**
   - Break into 20 PRs (1 per week × 20 weeks)
   - Each PR = 1 complete feature
   - CodeRabbit reviews every PR

3. **Phase 1 Development**
   - Start with exercise library (638 exercises)
   - Build workout logging flow
   - Implement smart features (plate calc, warm-ups)

4. **Iterate & Ship**
   - Weekly releases
   - User testing after each phase
   - Continuous feedback loop

---

**Ready to build the exact FORGE you envisioned?** 🚀
