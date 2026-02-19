# FORGE - Complete Design Specification

> **Status**: Design Phase - No implementation until approved
> **Version**: 1.0
> **Last Updated**: 2026-02-19

---

## Table of Contents

1. [Design Philosophy](#design-philosophy)
2. [User Personas](#user-personas)
3. [Core User Flows](#core-user-flows)
4. [Screen-by-Screen Specifications](#screen-by-screen-specifications)
5. [AI Integration](#ai-integration)
6. [Calendar Integration](#calendar-integration)
7. [Social Features](#social-features)
8. [Jeff Nippard Features](#jeff-nippard-features)
9. [Component Library](#component-library)
10. [Interaction Patterns](#interaction-patterns)
11. [Data Models](#data-models)

---

## Design Philosophy

### Core Principles

**1. Intelligence Without Intrusion**
- AI feels like a helpful coach, not a nagging robot
- Suggestions are timely and contextual, never pushy
- Users remain in control; AI assists, doesn't dictate

**2. Data-Driven Yet Motivating**
- MacroFactor's analytical depth with Strava's social energy
- Clear metrics and trends without overwhelming users
- Celebrate wins, learn from patterns

**3. Seamless Life Integration**
- Workout scheduling that respects your calendar
- Nutrition tracking that fits your lifestyle
- Social features that motivate without pressure

**4. Progressive Complexity**
- Simple for beginners (guided flows, templates)
- Powerful for advanced users (deep analytics, customization)
- Features reveal themselves as users grow

### Visual Language

**Color Palette**
- Primary: Deep Blue (#1E40AF) - Trust, strength, focus
- Secondary: Vibrant Orange (#F97316) - Energy, achievement
- Success: Green (#10B981) - Progress, growth
- Warning: Amber (#F59E0B) - Attention, caution
- Danger: Red (#EF4444) - Limits, errors
- Neutral: Slate scale (#0F172A to #F8FAFC) - Structure, readability

**Typography**
- Headings: Inter (Bold, 600-800 weight) - Modern, readable
- Body: Inter (Regular, 400-500 weight) - Clean, legible
- Data/Numbers: JetBrains Mono - Technical precision

**Spacing System**
- Base unit: 4px
- Scale: 4, 8, 12, 16, 24, 32, 48, 64, 96px
- Consistent padding/margins throughout

---

## User Personas

### 1. Alex - The Beginner
**Demographics**: 28, software engineer, sedentary job
**Goals**: Lose 20lbs, build healthy habits
**Pain Points**: Doesn't know what to eat, overwhelmed by gym routines
**Needs**: Guided onboarding, simple tracking, encouragement

### 2. Jordan - The Intermediate
**Demographics**: 32, has lifted for 2 years consistently
**Goals**: Break through plateau, optimize nutrition timing
**Pain Points**: Progress stalled, not sure if programming is optimal
**Needs**: Analytics, progressive overload tracking, TDEE adaptation

### 3. Sam - The Advanced
**Demographics**: 26, competitive powerlifter
**Goals**: Peak for competition, precise macro management
**Pain Points**: Needs granular control, tracks everything manually
**Needs**: Deep customization, advanced metrics, CSV export

### 4. Morgan - The Social Athlete
**Demographics**: 24, CrossFit enthusiast
**Goals**: Stay motivated, share achievements, find workout partners
**Pain Points**: Isolated training, lacks accountability
**Needs**: Social feed, challenges, clubs, messaging

---

## Core User Flows

### Flow 1: Onboarding Journey

**Goal**: Get user from download to first workout logged in under 10 minutes

#### Step 1: Welcome Screen
```
┌─────────────────────────────────┐
│         FORGE Logo              │
│                                 │
│   Build Your Best Self          │
│                                 │
│   [Get Started]                 │
│   [I already have an account]   │
└─────────────────────────────────┘
```

**Interactions**:
- Tapping "Get Started" → Account Creation
- Tapping "I already have an account" → Login

#### Step 2: Account Creation
```
┌─────────────────────────────────┐
│   ← Create Account              │
│                                 │
│   Email                         │
│   [___________________]         │
│                                 │
│   Username                      │
│   [___________________]         │
│                                 │
│   Password                      │
│   [___________________] 👁️      │
│   • At least 8 characters       │
│   • One uppercase               │
│   • One number                  │
│                                 │
│   [Continue]                    │
│                                 │
│   Already have account? Log in  │
└─────────────────────────────────┘
```

**Validation**:
- Email: Real-time check for valid format, existing accounts
- Username: 3-20 chars, alphanumeric + underscore, availability check
- Password: Strength meter (weak/fair/good/strong)
- Continue button disabled until all valid

#### Step 3: Profile Setup - Goals
```
┌─────────────────────────────────┐
│   Profile Setup (1/5)           │
│   ████░░░░░░░░░░░░░░░░░░       │
│                                 │
│   What's your primary goal?     │
│                                 │
│   ○ Lose Fat                    │
│   ○ Build Muscle                │
│   ○ Maintain / Recomp           │
│   ○ Athletic Performance        │
│   ○ General Health              │
│                                 │
│   [Continue]                    │
│   Skip for now                  │
└─────────────────────────────────┘
```

**AI Context**: Selection trains initial recommendation model

#### Step 4: Profile Setup - Measurements
```
┌─────────────────────────────────┐
│   Profile Setup (2/5)           │
│   ████████░░░░░░░░░░░░░░       │
│                                 │
│   Basic Info                    │
│                                 │
│   Age                           │
│   [25] years                    │
│                                 │
│   Sex                           │
│   ○ Male  ○ Female  ○ Other     │
│                                 │
│   Height                        │
│   [175] cm  / [5'9"] ft/in      │
│                                 │
│   Current Weight                │
│   [75] kg  / [165] lbs          │
│                                 │
│   [Continue]                    │
│   Skip for now                  │
└─────────────────────────────────┘
```

**Smart Features**:
- Unit toggle (metric/imperial) persists
- Auto-calculate BMI (shown subtly, not judgmental)

#### Step 5: Profile Setup - Activity Level
```
┌─────────────────────────────────┐
│   Profile Setup (3/5)           │
│   ████████████░░░░░░░░░░       │
│                                 │
│   Activity Level                │
│                                 │
│   ○ Sedentary                   │
│     Desk job, minimal exercise  │
│                                 │
│   ● Lightly Active              │
│     Exercise 1-3 days/week      │
│                                 │
│   ○ Moderately Active           │
│     Exercise 3-5 days/week      │
│                                 │
│   ○ Very Active                 │
│     Exercise 6-7 days/week      │
│                                 │
│   ○ Extremely Active            │
│     Athlete / physical job      │
│                                 │
│   [Continue]                    │
└─────────────────────────────────┘
```

**AI Calculation**: Initial TDEE using Mifflin-St Jeor equation

#### Step 6: Profile Setup - Target Weight
```
┌─────────────────────────────────┐
│   Profile Setup (4/5)           │
│   ████████████████░░░░░░       │
│                                 │
│   Target Weight                 │
│                                 │
│   [70] kg  / [154] lbs          │
│                                 │
│   Rate of Change                │
│   ○ Slow (0.25kg/week)          │
│     Preserve muscle, sustainable│
│   ● Moderate (0.5kg/week)       │
│     Balanced approach           │
│   ○ Fast (1kg/week)             │
│     Aggressive, harder to sustain│
│                                 │
│   Your estimated TDEE: 2,450 cal│
│   Recommended intake: 2,100 cal │
│   (to lose 0.5kg/week)          │
│                                 │
│   [Continue]                    │
└─────────────────────────────────┘
```

**Smart Warnings**:
- If rate too aggressive: "⚠️ This may lead to muscle loss"
- If target unrealistic: "⚠️ Recommended max: [calculated safe target]"

#### Step 7: Profile Setup - Calendar Integration
```
┌─────────────────────────────────┐
│   Profile Setup (5/5)           │
│   ████████████████████████     │
│                                 │
│   🗓️ Connect Your Calendar      │
│                                 │
│   Let FORGE suggest optimal     │
│   workout times based on your   │
│   schedule.                     │
│                                 │
│   [🔗 Connect Google Calendar]  │
│   [📅 Connect Outlook Calendar] │
│   [🍎 Connect Apple Calendar]   │
│                                 │
│   Skip for now                  │
│                                 │
│   ───────────────────────────   │
│                                 │
│   🔒 We only read your          │
│   availability. Calendar        │
│   events remain private.        │
└─────────────────────────────────┘
```

**Privacy First**:
- Clear explanation of what's accessed
- Easy to disconnect later
- Works fine without calendar

#### Step 8: First Workout Template
```
┌─────────────────────────────────┐
│   🎉 You're All Set!            │
│                                 │
│   Based on your goals, here's   │
│   a starter routine:            │
│                                 │
│   ╔═══════════════════════════╗ │
│   ║ Upper Body - Push         ║ │
│   ║ 45 min • Beginner         ║ │
│   ╠═══════════════════════════╣ │
│   ║ • Bench Press 3x8         ║ │
│   ║ • Overhead Press 3x10     ║ │
│   ║ • Tricep Pushdown 3x12    ║ │
│   ║ • Lateral Raises 3x15     ║ │
│   ╚═══════════════════════════╝ │
│                                 │
│   [Start This Workout Now]      │
│   [Browse More Routines]        │
│   [Skip - Take Me to Dashboard] │
└─────────────────────────────────┘
```

**Smart Matching**:
- AI selects routine based on goal, experience, equipment access
- Can customize later
- Non-intimidating volume for beginners

---

### Flow 2: Logging a Workout

**Goal**: Frictionless workout logging, under 30 seconds per exercise

#### Pre-Workout: Starting a Session
```
┌─────────────────────────────────┐
│   🏋️ Workouts                   │
│                                 │
│   Today - Wednesday, Feb 19     │
│                                 │
│   📅 SCHEDULED (from calendar)  │
│   ╔═══════════════════════════╗ │
│   ║ 6:00 PM - 7:00 PM         ║ │
│   ║ Push Day A                ║ │
│   ║ Chest, Shoulders, Triceps ║ │
│   ║                           ║ │
│   ║ [Start Workout]           ║ │
│   ╚═══════════════════════════╝ │
│                                 │
│   OR                            │
│                                 │
│   [+ Quick Start Empty Workout] │
│   [Browse Workout Templates]    │
│                                 │
│   ──────────────────────────    │
│   Recent Workouts               │
│   • Push Day A - 2 days ago     │
│   • Pull Day B - 4 days ago     │
│   • Leg Day - 6 days ago        │
└─────────────────────────────────┘
```

**Smart Features**:
- Calendar-scheduled workouts promoted at top
- "Quick Start" for spontaneous training
- Recent workouts for easy repeats

#### During Workout: Exercise List
```
┌─────────────────────────────────┐
│ ← Push Day A          ⏱️ 12:34  │
│                        [Finish]  │
│                                 │
│ ✅ Bench Press                  │
│    3 sets • 225 lbs PR!         │
│                                 │
│ ▶️  Incline Dumbbell Press      │
│    0/3 sets • Last: 70 lbs      │
│    [START SET]                  │
│                                 │
│ ⚪ Overhead Press                │
│    0/3 sets • Last: 135 lbs     │
│                                 │
│ ⚪ Lateral Raises                │
│    0/3 sets • Last: 25 lbs      │
│                                 │
│ ⚪ Tricep Pushdowns              │
│    0/3 sets • Last: 70 lbs      │
│                                 │
│ [+ Add Exercise]                │
│                                 │
│ ──────────────────────────────  │
│ Notes: Felt strong today 💪     │
└─────────────────────────────────┘
```

**Visual Hierarchy**:
- ✅ = Complete (green checkmark)
- ▶️ = In progress (orange play icon)
- ⚪ = Not started (gray circle)
- Timer always visible (top right)
- "Last" weight auto-populated for progression

#### Logging a Set
```
┌─────────────────────────────────┐
│   Incline Dumbbell Press        │
│                                 │
│   Set 1 of 3                    │
│                                 │
│   Weight (lbs)                  │
│   ┌───────────────────────────┐ │
│   │      [  70  ]             │ │
│   │   [-5]  [-1]  [+1]  [+5]  │ │
│   └───────────────────────────┘ │
│                                 │
│   Reps                          │
│   ┌───────────────────────────┐ │
│   │      [  10  ]             │ │
│   │   [-1]        [+1]        │ │
│   └───────────────────────────┘ │
│                                 │
│   🎯 Target: 8-12 reps          │
│   📊 Last set: 70 lbs × 10      │
│                                 │
│   [✓ Log Set]                   │
│   [Skip Set]                    │
│                                 │
│   ⏱️ Rest Timer: 90 seconds     │
│   [Start Timer]                 │
└─────────────────────────────────┘
```

**Interaction Magic**:
- Large touch targets (48px minimum)
- Quick increment buttons (-5, -1, +1, +5)
- Number pad appears on tap for manual entry
- "Log Set" → Auto-starts rest timer
- Haptic feedback on button press
- Previous set data shown for reference

#### Rest Timer
```
┌─────────────────────────────────┐
│                                 │
│         ⏱️ Rest Timer            │
│                                 │
│            01:23                │
│                                 │
│   ████████████████░░░░░░       │
│                                 │
│   [Skip Rest]  [Add 30s]        │
│                                 │
│   Next: Set 2 of 3              │
│   Incline Dumbbell Press        │
└─────────────────────────────────┘
```

**Timer Features**:
- Full-screen overlay (can't accidentally dismiss)
- Progress bar for visual feedback
- Subtle sound at 10s, 5s, 0s (can disable)
- "Add 30s" if need more recovery
- "Skip Rest" to proceed immediately

#### Post-Workout Summary
```
┌─────────────────────────────────┐
│   🎉 Workout Complete!          │
│                                 │
│   ⏱️  Duration: 52 minutes       │
│   🏋️  Total Volume: 12,450 lbs  │
│   💪 Exercises: 5                │
│   ✅ Sets Completed: 15          │
│                                 │
│   🏆 New Personal Records!      │
│   • Bench Press: 225 lbs × 5    │
│   • Overhead Press: 145 lbs × 8 │
│                                 │
│   How did it feel?              │
│   😫 💪 😐 💪 🔥                 │
│   (Tap to rate difficulty)      │
│                                 │
│   Notes (optional)              │
│   [___________________________] │
│                                 │
│   📸 Add Progress Photo          │
│                                 │
│   ✓ Share to Feed               │
│   (Only friends can see)        │
│                                 │
│   [Done]                        │
└─────────────────────────────────┘
```

**Celebration**:
- Confetti animation for PRs
- Stats make user feel accomplished
- RPE (difficulty) capture for AI learning
- Social sharing opt-in (not forced)

---

### Flow 3: Logging Nutrition

**Goal**: Under 20 seconds to log a meal, with multiple input methods

#### Main Nutrition Screen
```
┌─────────────────────────────────┐
│   🍽️ Nutrition    Wed, Feb 19   │
│                                 │
│   Daily Target: 2,100 calories  │
│                                 │
│   ╔═══════════════════════════╗ │
│   ║     Calories                ║
│   ║     1,534 / 2,100           ║
│   ║  ██████████████░░░░░ 73%    ║
│   ╚═══════════════════════════╝ │
│                                 │
│   Macros                        │
│   Protein    Fat      Carbs     │
│   128/160g   45/70g   180/210g  │
│   ████░░     ████░░   ██████░   │
│   80%        64%      86%       │
│                                 │
│   ──────────────────────────    │
│                                 │
│   🌅 BREAKFAST (420 cal)        │
│   • Oatmeal, 100g               │
│   • Banana, medium              │
│   • Whey protein, 1 scoop       │
│                        [Edit]   │
│                                 │
│   🌞 LUNCH (680 cal)            │
│   • Grilled chicken, 200g       │
│   • Brown rice, 150g            │
│   • Broccoli, 100g              │
│                        [Edit]   │
│                                 │
│   🍴 SNACK (434 cal)            │
│   • Greek yogurt, 200g          │
│   • Mixed nuts, 30g             │
│                        [Edit]   │
│                                 │
│   🌙 DINNER (0 cal)             │
│   [+ Add Food]                  │
│                                 │
│   ──────────────────────────    │
│                                 │
│   [📸 Scan Food] [🔍 Search]    │
└─────────────────────────────────┘
```

**Visual Design**:
- Color-coded macro bars (Protein=orange, Fat=yellow, Carbs=blue)
- Meal icons for visual scanning
- Remaining calories prominent
- Quick "Add Food" per meal

#### Method 1: AI Food Recognition
```
┌─────────────────────────────────┐
│ ← Scan Food                     │
│                                 │
│  ┌───────────────────────────┐  │
│  │                           │  │
│  │      [Camera Viewfinder]  │  │
│  │                           │  │
│  │     Aim at your meal      │  │
│  │                           │  │
│  └───────────────────────────┘  │
│                                 │
│  [📸 Take Photo]  [🖼️ Gallery]  │
│                                 │
│  Tips for best results:         │
│  • Good lighting                │
│  • Separate items visible       │
│  • Include size reference       │
│    (fork, hand, etc.)           │
└─────────────────────────────────┘
```

**After photo taken:**
```
┌─────────────────────────────────┐
│ ← Analyzing Meal... 🤖          │
│                                 │
│  ┌───────────────────────────┐  │
│  │   [Photo of food]         │  │
│  └───────────────────────────┘  │
│                                 │
│  AI Detected:                   │
│  ✓ Grilled chicken breast       │
│  ✓ Brown rice                   │
│  ✓ Steamed broccoli             │
│  ✓ Olive oil (estimated)        │
│                                 │
│  Estimated portions:            │
│  • Chicken: 200g                │
│  • Rice: 150g                   │
│  • Broccoli: 100g               │
│  • Olive oil: 1 tbsp            │
│                                 │
│  Total: 520 calories            │
│  P: 48g | F: 12g | C: 55g       │
│                                 │
│  [✓ Looks Good] [✏️ Adjust]     │
└─────────────────────────────────┘
```

**Smart Features**:
- ONNX model for fast recognition
- GPT-4V fallback for complex meals
- Portion estimation from visual cues
- User can adjust before logging

#### Method 2: Quick Search
```
┌─────────────────────────────────┐
│ ← Add Food                      │
│                                 │
│  🔍 [grilled chicken]           │
│                                 │
│  Results:                       │
│                                 │
│  🥇 Recent                       │
│  ┌─────────────────────────┐   │
│  │ Grilled Chicken Breast  │   │
│  │ 200g • 330 cal          │   │
│  │ P: 62g | F: 7g | C: 0g  │   │
│  └─────────────────────────┘   │
│                                 │
│  🗂️ USDA Database               │
│  ┌─────────────────────────┐   │
│  │ Chicken, broiled        │   │
│  │ Per 100g • 165 cal      │   │
│  └─────────────────────────┘   │
│  ┌─────────────────────────┐   │
│  │ Chicken breast, grilled │   │
│  │ Per 100g • 165 cal      │   │
│  └─────────────────────────┘   │
│                                 │
│  [Create Custom Food]           │
└─────────────────────────────────┘
```

**Search Intelligence**:
- Recent foods at top (frequently logged)
- USDA database results
- Custom foods (user-created)
- Barcode scanner option

#### Method 3: Quick Add (Macro-based)
```
┌─────────────────────────────────┐
│ ← Quick Add Calories            │
│                                 │
│  For tracking without details   │
│                                 │
│  Calories                       │
│  [500]                          │
│                                 │
│  Macros (optional)              │
│  Protein  [40] g                │
│  Fat      [15] g                │
│  Carbs    [50] g                │
│                                 │
│  = 490 calories                 │
│  (Close to 500 ✓)               │
│                                 │
│  Meal                           │
│  ○ Breakfast                    │
│  ○ Lunch                        │
│  ● Dinner                       │
│  ○ Snack                        │
│                                 │
│  Notes (optional)               │
│  [Restaurant meal, estimated]   │
│                                 │
│  [Add to Log]                   │
└─────────────────────────────────┘
```

**Use Case**: Quick tracking when details unknown (e.g., restaurant meal)

#### Creating Custom Food
```
┌─────────────────────────────────┐
│ ← Create Custom Food            │
│                                 │
│  Food Name                      │
│  [My Protein Smoothie]          │
│                                 │
│  Serving Size                   │
│  [1] [cup ▼]                    │
│                                 │
│  Per Serving:                   │
│  Calories    [320]              │
│  Protein     [35] g             │
│  Fat         [8] g              │
│  Carbs       [30] g             │
│  Fiber       [5] g (optional)   │
│                                 │
│  ✓ Save to My Foods             │
│                                 │
│  [Create]                       │
└─────────────────────────────────┘
```

**Persistence**: Saved to "My Foods" for quick access later

---

### Flow 4: AI-Assisted Calendar Scheduling

**Goal**: FORGE becomes your workout scheduling assistant

#### Initial Calendar Connection
```
┌─────────────────────────────────┐
│   🗓️ Smart Scheduling           │
│                                 │
│   FORGE can analyze your        │
│   calendar and automatically    │
│   suggest the best times to     │
│   train each week.              │
│                                 │
│   What we'll do:                │
│   ✓ Read your availability      │
│   ✓ Avoid conflicts             │
│   ✓ Optimize for recovery       │
│   ✓ Adapt to changes            │
│                                 │
│   What we WON'T do:             │
│   ✗ Read event details          │
│   ✗ Modify your calendar        │
│   ✗ Share your schedule         │
│                                 │
│   [Connect Google Calendar]     │
│   [Connect Outlook]             │
│   [Maybe Later]                 │
└─────────────────────────────────┘
```

#### AI Analysis in Progress
```
┌─────────────────────────────────┐
│   🤖 Analyzing Your Schedule... │
│                                 │
│   ████████████░░░░░░░░░░       │
│                                 │
│   • Reading availability        │
│   • Finding optimal windows     │
│   • Checking recovery needs     │
│   • Building schedule           │
│                                 │
│   This may take 10-15 seconds   │
└─────────────────────────────────┘
```

#### AI Schedule Proposal
```
┌─────────────────────────────────┐
│ ← Proposed Schedule             │
│                                 │
│   Based on your calendar, here  │
│   are the best workout times:   │
│                                 │
│   📅 THIS WEEK (Feb 19-25)      │
│                                 │
│   MON • Push Day A              │
│   🕐 6:00 AM - 7:00 AM          │
│   Free slot before work         │
│   ✓ Well-rested (weekend)       │
│                                 │
│   WED • Pull Day A              │
│   🕕 6:00 PM - 7:00 PM          │
│   After work, before dinner     │
│   ✓ 48hr recovery from Monday   │
│                                 │
│   FRI • Leg Day                 │
│   🕐 6:00 AM - 7:00 AM          │
│   Free morning slot             │
│   ✓ Weekend ahead for recovery │
│                                 │
│   SAT • Push Day B              │
│   🕙 10:00 AM - 11:00 AM        │
│   Late morning (no conflicts)   │
│   ✓ Extra recovery time         │
│                                 │
│   🔄 4 workouts/week             │
│   💪 Upper/Lower split           │
│   ⏰ Avg. duration: 60 min       │
│                                 │
│   [Accept Schedule]             │
│   [Customize]                   │
└─────────────────────────────────┘
```

**AI Logic**:
- Finds 60-90min blocks with no conflicts
- Respects minimum recovery (48hr between same muscle groups)
- Prefers consistent times (builds habit)
- Avoids late nights if early morning commitments next day

#### Customizing AI Schedule
```
┌─────────────────────────────────┐
│ ← Customize Schedule            │
│                                 │
│   Weekly Workout Goal           │
│   [4] workouts per week         │
│   (Min: 2, Max: 6)              │
│                                 │
│   Preferred Times               │
│   ✓ Early Morning (6-9 AM)      │
│   ✗ Midday (12-2 PM)            │
│   ✓ Evening (5-8 PM)            │
│   ✗ Night (8-11 PM)             │
│                                 │
│   Rest Day Preferences          │
│   ✓ Always rest on Sunday       │
│   ✗ Prefer consecutive rest days│
│                                 │
│   Travel / Busy Periods         │
│   [+ Add Blocked Dates]         │
│   • Mar 15-20 (Business trip)   │
│                                 │
│   Workout Duration              │
│   ○ Quick (30-45 min)           │
│   ● Standard (60 min)           │
│   ○ Extended (90+ min)          │
│                                 │
│   [Generate New Schedule]       │
└─────────────────────────────────┘
```

**Flexibility**: User controls constraints, AI optimizes within them

#### Calendar View with Workouts
```
┌─────────────────────────────────┐
│   📅 This Week                  │
│                                 │
│   MON 19                        │
│   🕐 6:00 AM - 7:00 AM          │
│   💪 Push Day A                 │
│   ─                             │
│   🕘 9:00 AM - 10:00 AM         │
│   📊 Team Standup               │
│   ─                             │
│   🕐 1:00 PM - 2:00 PM          │
│   💼 Client Meeting             │
│                                 │
│   TUE 20                        │
│   🕘 9:00 AM - 5:00 PM          │
│   💼 Conference (all day)       │
│   ─                             │
│   [AI suggests rest day]        │
│                                 │
│   WED 21                        │
│   🕕 6:00 PM - 7:00 PM          │
│   💪 Pull Day A [SCHEDULED]     │
│                                 │
│   THU 22                        │
│   🕑 2:00 PM - 3:00 PM          │
│   💼 1-on-1 with Manager        │
│   ─                             │
│   [+ Add Workout]               │
│                                 │
│   [Week View] [Month View]      │
└─────────────────────────────────┘
```

**Integration**:
- Calendar events shown with icons (💼 work, 💪 workout, 🍽️ meal, etc.)
- AI-scheduled workouts highlighted
- Can manually add/move workouts
- Syncs back to Google Calendar (optional)

#### AI Rescheduling on Conflict
```
┌─────────────────────────────────┐
│   ⚠️ Schedule Conflict Detected │
│                                 │
│   Your Wednesday 6 PM workout   │
│   conflicts with a new calendar │
│   event:                        │
│                                 │
│   🍽️ Dinner with Client         │
│   Wed, Feb 21 • 6:00-8:00 PM    │
│                                 │
│   Would you like me to          │
│   reschedule your workout?      │
│                                 │
│   Suggested alternatives:       │
│                                 │
│   ○ Wed, 6:00 AM (same day)     │
│     Morning workout instead     │
│                                 │
│   ● Thu, 6:00 PM (next day)     │
│     Moves rest day to Wed       │
│                                 │
│   ○ Skip this week              │
│     Maintain 3 workouts         │
│                                 │
│   [Reschedule] [Ignore]         │
└─────────────────────────────────┘
```

**Proactive AI**: Detects conflicts, suggests solutions automatically

---

### Flow 5: Social Features - Building Community

**Goal**: Strava-like motivation through community

#### Social Feed (Home)
```
┌─────────────────────────────────┐
│   🏠 Feed          🔍  👤  ⚙️    │
│                                 │
│   ┌─────────────────────────┐  │
│   │ @alex_lifts  •  2h ago  │  │
│   │ ────────────────────────│  │
│   │ Completed: Push Day A   │  │
│   │ 52 min • 12,450 lbs     │  │
│   │                         │  │
│   │ 💬 8  👍 24  🔥 3        │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ @jordan_fit  •  4h ago  │  │
│   │ ────────────────────────│  │
│   │ 🏆 New PR!              │  │
│   │ Deadlift: 405 lbs × 1   │  │
│   │                         │  │
│   │ [Progress photo]        │  │
│   │                         │  │
│   │ "Finally hit 4 plates!" │  │
│   │                         │  │
│   │ 💬 15  👍 48  🔥 12      │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ @sam_strong  •  6h ago  │  │
│   │ ────────────────────────│  │
│   │ 💪 Weekly streak: 5     │  │
│   │ Workout frequency up    │  │
│   │ 20% this month!         │  │
│   │                         │  │
│   │ 💬 3  👍 12              │  │
│   └─────────────────────────┘  │
│                                 │
│   [Load More]                   │
└─────────────────────────────────┘
```

**Feed Algorithm**:
- Friends' workouts & achievements
- Workout completions, PRs, streaks, photos
- Chronological (no manipulative algorithm)
- Reactions: 💬 Comment, 👍 Kudos, 🔥 Fire (exceptional)

#### Workout Detail from Feed
```
┌─────────────────────────────────┐
│ ← @alex_lifts                   │
│                                 │
│   Posted 2 hours ago            │
│                                 │
│   💪 Push Day A                 │
│   52 minutes • 12,450 lbs       │
│                                 │
│   Exercises:                    │
│   • Bench Press: 225×5 🏆 PR!   │
│   • Incline DB Press: 70×10×3   │
│   • Overhead Press: 145×8 🏆 PR!│
│   • Lateral Raises: 25×15×3     │
│   • Tricep Pushdown: 70×12×3    │
│                                 │
│   Notes: "Felt strong today!"   │
│                                 │
│   ──────────────────────────    │
│                                 │
│   💬 8 Comments                 │
│                                 │
│   @jordan_fit: "Beast mode! 💪" │
│   2h ago • 👍 3                  │
│                                 │
│   @sam_strong: "Nice PRs!"      │
│   1h ago • 👍 1                  │
│                                 │
│   You: [Add a comment...]       │
│                                 │
│   ──────────────────────────    │
│                                 │
│   👍 24  🔥 3                    │
│   [Give Kudos] [React]          │
└─────────────────────────────────┘
```

**Privacy Controls**:
- Can toggle visibility per workout (public/friends/private)
- Default configurable in settings

#### Finding Friends
```
┌─────────────────────────────────┐
│ ← Find Friends                  │
│                                 │
│   🔍 [Search by username...]    │
│                                 │
│   📱 SUGGESTED FROM CONTACTS    │
│   ┌─────────────────────────┐  │
│   │ @jordan_fit             │  │
│   │ Jordan Smith            │  │
│   │ 48 mutual friends       │  │
│   │              [Follow]   │  │
│   └─────────────────────────┘  │
│                                 │
│   👥 POPULAR IN YOUR AREA       │
│   ┌─────────────────────────┐  │
│   │ @alex_lifts             │  │
│   │ Alex Johnson            │  │
│   │ 15 followers            │  │
│   │              [Follow]   │  │
│   └─────────────────────────┘  │
│                                 │
│   🏋️ SIMILAR GOALS              │
│   ┌─────────────────────────┐  │
│   │ @sam_strong             │  │
│   │ Sam Davis               │  │
│   │ Also training for       │  │
│   │ strength gains          │  │
│   │              [Follow]   │  │
│   └─────────────────────────┘  │
│                                 │
│   [Invite Friends via Email]    │
└─────────────────────────────────┘
```

**Discovery**:
- Contact sync (opt-in, privacy-first)
- Location-based suggestions
- Goal/interest matching
- Mutual friend suggestions

#### Clubs Feature
```
┌─────────────────────────────────┐
│   🏘️ Clubs                       │
│                                 │
│   YOUR CLUBS                    │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 💪 Morning Warriors      │  │
│   │ 127 members             │  │
│   │ Early AM training crew  │  │
│   │                         │  │
│   │ 🏆 Monthly Challenge:   │  │
│   │ "Bench 1,000,000 lbs"   │  │
│   │ Progress: 67% (670K)    │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 🏋️ Powerlifting Club     │  │
│   │ 543 members             │  │
│   │ Big 3 focused           │  │
│   │                         │  │
│   │ 📊 Leaderboard:         │  │
│   │ You're #12 this month   │  │
│   └─────────────────────────┘  │
│                                 │
│   DISCOVER CLUBS                │
│   [Browse All] [Create Club]    │
└─────────────────────────────────┘
```

**Club Features**:
- Shared challenges (total volume, frequency, etc.)
- Leaderboards (opt-in, friendly competition)
- Club feed (only club members see)
- Private or public clubs

#### Club Detail
```
┌─────────────────────────────────┐
│ ← Morning Warriors              │
│                                 │
│   💪 Early AM training crew     │
│   127 members                   │
│                                 │
│   [📊 Leaderboard] [👥 Members] │
│   [💬 Chat] [🏆 Challenges]     │
│                                 │
│   ──────────────────────────    │
│   ACTIVE CHALLENGE              │
│                                 │
│   Bench Press 1,000,000 lbs     │
│   Ends in 12 days               │
│                                 │
│   Progress: 67% (670,450 lbs)   │
│   ████████████████░░░░░░       │
│                                 │
│   Your contribution: 12,450 lbs │
│   Rank: #8 of 127               │
│                                 │
│   ──────────────────────────    │
│   RECENT ACTIVITY               │
│                                 │
│   @alex_lifts benched 225×5     │
│   2h ago • +1,125 lbs           │
│                                 │
│   @jordan_fit benched 315×3     │
│   4h ago • +945 lbs 🔥          │
│                                 │
│   [View Full Feed]              │
└─────────────────────────────────┘
```

**Gamification**:
- Collective goals (builds camaraderie)
- Individual contributions visible
- Friendly competition without pressure

#### Direct Messaging
```
┌─────────────────────────────────┐
│   💬 Messages                   │
│                                 │
│   ┌─────────────────────────┐  │
│   │ @jordan_fit             │  │
│   │ "Thanks for the tips!"  │  │
│   │ 1h ago               •  │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ @alex_lifts             │  │
│   │ "See you at the gym!"   │  │
│   │ 3h ago                  │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ Morning Warriors        │  │
│   │ @sam: "Who's training"  │  │
│   │ "tomorrow?"             │  │
│   │ 5h ago               •  │  │
│   └─────────────────────────┘  │
│                                 │
│   [+ New Message]               │
└─────────────────────────────────┘
```

**Messaging**:
- 1-on-1 DMs
- Club group chats
- Share workouts, routines, tips

---

## Screen-by-Screen Specifications

### Dashboard (Main Home)

**Purpose**: Daily command center for training and nutrition

```
┌─────────────────────────────────┐
│   FORGE                    ⚙️👤 │
│                                 │
│   Good evening, Alex! 👋        │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 🗓️ TODAY'S SCHEDULE      │  │
│   │                         │  │
│   │ 6:00 PM - Push Day A    │  │
│   │ [Start Workout]         │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 🍽️ NUTRITION TODAY      │  │
│   │                         │  │
│   │ 1,534 / 2,100 cal       │  │
│   │ ██████████████░░░░ 73%  │  │
│   │                         │  │
│   │ P: 128/160g  F: 45/70g  │  │
│   │ C: 180/210g             │  │
│   │                         │  │
│   │ [Quick Add] [Scan Food] │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 📊 THIS WEEK            │  │
│   │                         │  │
│   │ Workouts: 3/4 ✓         │  │
│   │ Weight: 74.8 kg (↓0.2)  │  │
│   │ Avg calories: 2,050     │  │
│   │                         │  │
│   │ [View Details]          │  │
│   └─────────────────────────┘  │
│                                 │
│   ┌─────────────────────────┐  │
│   │ 💡 AI INSIGHTS          │  │
│   │                         │  │
│   │ "You're on track for    │  │
│   │ 0.5kg loss this week.   │  │
│   │ Consider adding 10g     │  │
│   │ protein at dinner."     │  │
│   │                         │  │
│   │ [Learn More]            │  │
│   └─────────────────────────┘  │
└─────────────────────────────────┘
```

**Key Features**:
- Time-aware greeting (Good morning/afternoon/evening)
- Calendar-scheduled workout prominently displayed
- Nutrition at-a-glance
- Weekly summary
- Actionable AI insights (not just stats)

---

### Progress Screen

**Purpose**: Visualize long-term trends and celebrate wins

```
┌─────────────────────────────────┐
│ ← Progress                      │
│                                 │
│   [Weight] [Body] [Strength]    │
│                                 │
│   ──── WEIGHT TRACKING ────     │
│                                 │
│   Current: 74.8 kg              │
│   Start: 78.0 kg (Jan 1)        │
│   Change: -3.2 kg in 7 weeks    │
│                                 │
│   ┌─────────────────────────┐  │
│   │    Weight Trend         │  │
│   │                         │  │
│   │ 78kg ●                  │  │
│   │      ●                  │  │
│   │       ●──●              │  │
│   │           ●──●          │  │
│   │               ●──● 74.8│  │
│   │                         │  │
│   │ Jan    Feb    Mar       │  │
│   └─────────────────────────┘  │
│                                 │
│   Daily Weigh-ins: 48/49 ✓      │
│   (98% consistency)             │
│                                 │
│   Trend: -0.46 kg/week          │
│   🎯 Target: -0.5 kg/week       │
│   ✅ On track!                  │
│                                 │
│   [Log Weight Today]            │
│                                 │
│   ──── BODY MEASUREMENTS ────   │
│   [+ Add Progress Photo]        │
│   [View Photo Timeline]         │
└─────────────────────────────────┘
```

**Smart Features**:
- EWMA trend line (smooths daily fluctuations)
- Consistency tracking (gamifies daily weigh-ins)
- Target comparison (visual feedback)

---

### Strength Progress
```
┌─────────────────────────────────┐
│ ← Strength Progress             │
│                                 │
│   Select Exercise:              │
│   [Bench Press ▼]               │
│                                 │
│   ┌─────────────────────────┐  │
│   │  Bench Press 1RM        │  │
│   │                         │  │
│   │ 250 ●                   │  │
│   │     ●                   │  │
│   │      ●                  │  │
│   │       ●──● 225 (current)│  │
│   │                         │  │
│   │ Jan    Feb    Mar       │  │
│   └─────────────────────────┘  │
│                                 │
│   Estimated 1RM: 225 lbs        │
│   (from 205×6 on Feb 17)        │
│                                 │
│   Recent PRs:                   │
│   • Feb 19: 225×5 🔥            │
│   • Feb 12: 220×6               │
│   • Feb 5:  205×8               │
│                                 │
│   Volume Trend (4 weeks):       │
│   ┌─────────────────────────┐  │
│   │ Week 1: 8,500 lbs       │  │
│   │ Week 2: 9,200 lbs       │  │
│   │ Week 3: 9,800 lbs       │  │
│   │ Week 4: 10,500 lbs ✓    │  │
│   └─────────────────────────┘  │
│                                 │
│   💡 Strength improving 2.1%/wk │
└─────────────────────────────────┘
```

**Analytics**:
- Estimated 1RM from all working sets
- PR history
- Volume progression (total lbs lifted)
- Rate of improvement

---

### Profile & Settings

```
┌─────────────────────────────────┐
│ ← Profile                       │
│                                 │
│   ┌─────────────────────────┐  │
│   │   [Profile Photo]       │  │
│   │                         │  │
│   │   @alex_lifts           │  │
│   │   Alex Johnson          │  │
│   │                         │  │
│   │   127 followers         │  │
│   │   98 following          │  │
│   └─────────────────────────┘  │
│                                 │
│   [Edit Profile]                │
│                                 │
│   ──────────────────────────    │
│   ACCOUNT                       │
│   Email                         │
│   Password                      │
│   Connected Accounts            │
│   • Google Calendar ✓           │
│   • Strava (Not connected)      │
│                                 │
│   ──────────────────────────    │
│   PREFERENCES                   │
│   Units (Metric / Imperial)     │
│   Theme (Dark / Light / Auto)   │
│   Notifications                 │
│   Privacy                       │
│                                 │
│   ──────────────────────────    │
│   DATA                          │
│   Export All Data               │
│   Delete Account                │
│                                 │
│   [Log Out]                     │
└─────────────────────────────────┘
```

---

## AI Integration

### AI Touchpoints Throughout FORGE

#### 1. Food Recognition (Vision Model)
**Tech Stack**: ONNX Runtime + GPT-4V fallback

**Flow**:
1. User takes photo of meal
2. Image preprocessed (resize, normalize, tensor conversion)
3. Local ONNX model runs inference (< 2 seconds)
4. If confidence > 80%, return results
5. If confidence < 80%, send to GPT-4V API
6. Parse nutrition data, present to user

**Accuracy Goals**:
- Single foods: 90%+ accuracy
- Simple meals (3-4 items): 80%+ accuracy
- Complex meals: 70%+ accuracy (with GPT-4V)

**User Experience**:
```
📸 Photo taken
   ↓
🤖 Analyzing... (1-2s local, 3-5s cloud)
   ↓
✅ "Detected: Grilled chicken, brown rice, broccoli"
   ↓
✏️ User adjusts portions if needed
   ↓
💾 Logged to diary
```

#### 2. Adaptive TDEE (Statistical Model)
**Algorithm**: Mifflin-St Jeor + Weekly Adjustment

**Inputs**:
- Daily weight (EWMA trend)
- Daily calorie intake
- Weekly check-in (hunger, energy, performance)

**Logic**:
```python
# Week 1: Use Mifflin-St Jeor baseline
tdee = calculate_mifflin_st_jeor(age, sex, weight, height, activity)

# Week 2+: Adapt based on actual results
actual_weight_change = (current_weight - last_week_weight) * 7700  # calories
expected_change = (avg_intake - tdee) * 7
error = actual_weight_change - expected_change

# Adjust TDEE
tdee_adjusted = tdee + (error / 7)  # Smooth adjustment

# Update target
new_target = tdee_adjusted + calorie_delta_for_goal
```

**UI Presentation**:
```
┌─────────────────────────────────┐
│   📊 Weekly Check-In            │
│                                 │
│   Last week:                    │
│   • Avg intake: 2,050 cal       │
│   • Weight change: -0.4 kg      │
│   • Expected: -0.5 kg           │
│                                 │
│   🤖 AI Analysis:               │
│   "Your metabolism is slightly  │
│   higher than predicted. I'm    │
│   increasing your target by     │
│   50 calories to 2,150/day."    │
│                                 │
│   How was your week?            │
│   Hunger:   😫 😐 😊 ✓          │
│   Energy:   😫 😐 😊 ✓          │
│   Workouts: 💪 💪 💪 💪 ✓        │
│                                 │
│   [Accept New Target]           │
│   [Adjust Manually]             │
└─────────────────────────────────┘
```

#### 3. Workout Recommendations (LLM-based)
**Trigger**: User asks "What should I train today?"

**Context Sent to LLM**:
```json
{
  "user_profile": {
    "goal": "build_muscle",
    "experience": "intermediate",
    "available_equipment": ["barbell", "dumbbells", "cables"]
  },
  "recent_workouts": [
    {"date": "2026-02-17", "type": "push", "muscle_groups": ["chest", "shoulders", "triceps"]},
    {"date": "2026-02-15", "type": "pull", "muscle_groups": ["back", "biceps"]}
  ],
  "calendar_availability": {
    "today": {"free_time": "6:00 PM - 7:30 PM"},
    "duration_available": 90
  },
  "recovery_status": {
    "chest": "recovered",
    "back": "recovering",
    "legs": "recovered"
  }
}
```

**LLM Prompt**:
```
You are a strength coach. Based on the user's profile, recent workouts, and recovery status, suggest today's optimal workout.

Format:
- Workout name (e.g., "Leg Day - Quad Focus")
- Target duration
- 4-6 exercises with sets/reps
- Brief rationale (1 sentence)

Be specific, actionable, and considerate of recovery.
```

**Response**:
```
┌─────────────────────────────────┐
│   🤖 AI Recommendation          │
│                                 │
│   Based on your schedule and    │
│   recovery, I suggest:          │
│                                 │
│   💪 Leg Day - Quad Focus       │
│   Duration: 75 minutes          │
│                                 │
│   1. Squat: 4×6-8               │
│   2. Bulgarian Split Squat: 3×10│
│   3. Leg Press: 3×12            │
│   4. Leg Extension: 3×15        │
│   5. Calf Raises: 4×20          │
│                                 │
│   Rationale: Your upper body    │
│   trained yesterday, legs are   │
│   fresh, and you have 90 min.   │
│                                 │
│   [Start This Workout]          │
│   [Modify]  [Suggest Different] │
└─────────────────────────────────┘
```

#### 4. Calendar-Aware Scheduling (Heuristic + LLM)
**Step 1**: Parse calendar for free blocks (heuristic)
**Step 2**: LLM optimizes workout placement

**Context to LLM**:
```json
{
  "user_preferences": {
    "workouts_per_week": 4,
    "preferred_times": ["early_morning", "evening"],
    "workout_duration": 60,
    "split_type": "upper_lower"
  },
  "free_blocks_this_week": [
    {"day": "Monday", "time": "6:00-7:30 AM", "duration": 90},
    {"day": "Monday", "time": "6:00-8:00 PM", "duration": 120},
    {"day": "Wednesday", "time": "6:00-7:00 PM", "duration": 60},
    {"day": "Friday", "time": "6:00-7:30 AM", "duration": 90},
    {"day": "Saturday", "time": "10:00-12:00 PM", "duration": 120}
  ],
  "optimal_recovery": 48  // hours between same muscle groups
}
```

**LLM Task**:
```
Schedule 4 workouts (Upper A, Lower A, Upper B, Lower B) into the available blocks.

Constraints:
- Prefer early morning or evening (per user preference)
- Minimum 48hr between same muscle groups
- Distribute evenly across week
- Avoid back-to-back days if possible

Return JSON:
[
  {"day": "Monday", "time": "6:00 AM", "workout": "Upper A"},
  ...
]
```

#### 5. Progress Insights (Statistical + LLM)
**Every Sunday**: AI generates weekly summary

**Data Aggregated**:
- Weight trend (EWMA)
- Average calories vs target
- Workout adherence (scheduled vs completed)
- Volume progression
- PR count

**LLM Summary Generation**:
```
User data for week of Feb 12-18:
- Weight: 75.2 → 74.8 kg (-0.4 kg)
- Target loss: -0.5 kg/week
- Avg calories: 2,050 (target: 2,100)
- Workouts completed: 4/4 (100%)
- Total volume: +8% vs last week
- PRs: 2 (Bench Press, Overhead Press)

Generate a motivating, concise summary (2-3 sentences) with one actionable insight.
```

**Output**:
```
┌─────────────────────────────────┐
│   📊 Weekly Summary             │
│                                 │
│   Great week, Alex! You hit all │
│   4 workouts and set 2 PRs.     │
│   Weight is trending down       │
│   steadily at -0.4kg. Consider  │
│   adding 50 calories to avoid   │
│   excessive loss.               │
│                                 │
│   🏆 Achievements:              │
│   • 100% workout adherence      │
│   • 8% volume increase          │
│   • 2 new personal records      │
│                                 │
│   💡 Next week's focus:         │
│   Maintain this consistency!    │
│                                 │
│   [View Details]                │
└─────────────────────────────────┘
```

---

## Calendar Integration

### Google Calendar OAuth Flow

#### Step 1: User Initiates Connection
```
Settings → Connected Accounts → Google Calendar → Connect
   ↓
OAuth popup (Google login)
   ↓
Request scopes: calendar.readonly, calendar.events.freebusy
   ↓
User approves
   ↓
Store refresh token (encrypted)
```

#### Step 2: Sync Calendar Events
**Frequency**: Every 6 hours, or on-demand

**API Call**:
```javascript
// Fetch free/busy for next 7 days
const freebusy = await calendar.freebusy.query({
  timeMin: startOfWeek,
  timeMax: endOfWeek,
  items: [{ id: 'primary' }]
})

// Parse busy blocks
const busyBlocks = freebusy.calendars.primary.busy
// → [{ start: '2026-02-19T09:00:00Z', end: '2026-02-19T10:00:00Z' }, ...]

// Find free blocks (inverse)
const freeBlocks = invertBusyBlocks(busyBlocks)
// → [{ start: '2026-02-19T06:00:00Z', end: '2026-02-19T09:00:00Z' }, ...]
```

#### Step 3: AI Schedules Workouts
**Heuristic Rules**:
1. Filter blocks by duration (>= 45 min)
2. Filter by user preferences (time of day)
3. Apply recovery constraints (48hr between same muscle groups)
4. Optimize for consistency (same time each week)

**LLM Refinement** (optional):
- Send filtered blocks + constraints to LLM
- LLM returns optimized schedule

#### Step 4: Present to User
```
┌─────────────────────────────────┐
│   🤖 Proposed Schedule          │
│                                 │
│   I found these optimal times:  │
│                                 │
│   MON • 6:00 AM - Upper A       │
│   WED • 6:00 PM - Lower A       │
│   FRI • 6:00 AM - Upper B       │
│   SAT • 10:00 AM - Lower B      │
│                                 │
│   [Accept] [Customize]          │
└─────────────────────────────────┘
```

#### Step 5: Add to Google Calendar (Optional)
**User Opt-in**:
```
Settings → Calendar Sync → ✓ Add workouts back to Google Calendar
```

**API Call**:
```javascript
await calendar.events.insert({
  calendarId: 'primary',
  resource: {
    summary: '💪 FORGE: Upper A',
    description: 'Push workout (Chest, Shoulders, Triceps)',
    start: { dateTime: '2026-02-19T06:00:00Z' },
    end: { dateTime: '2026-02-19T07:00:00Z' },
    reminders: {
      useDefault: false,
      overrides: [{ method: 'popup', minutes: 15 }]
    }
  }
})
```

**Result**: User sees FORGE workouts in Google Calendar, gets reminders

---

### Smart Conflict Detection

**Scenario**: User's calendar changes after schedule created

**Flow**:
1. Webhook from Google Calendar (or 6hr polling)
2. Detect new event conflicting with scheduled workout
3. Find alternative free blocks
4. Notify user with suggestion

**Notification**:
```
┌─────────────────────────────────┐
│   ⚠️ Schedule Update Needed     │
│                                 │
│   Your Wednesday 6 PM workout   │
│   conflicts with:               │
│   "Client Dinner" (6-8 PM)      │
│                                 │
│   Suggested alternatives:       │
│   ○ Wed, 6:00 AM (same day)     │
│   ● Thu, 6:00 PM (recovery OK)  │
│                                 │
│   [Reschedule] [Keep as-is]     │
└─────────────────────────────────┘
```

---

## Jeff Nippard Features

### 1. Science-Based Programs

**Program Library**:
```
┌─────────────────────────────────┐
│ ← Programs (Jeff Nippard)      │
│                                 │
│   🏋️ STRENGTH                   │
│   ┌─────────────────────────┐  │
│   │ Powerbuilding 2.0       │  │
│   │ 6 weeks • 4x/week       │  │
│   │ Intermediate-Advanced   │  │
│   │              [Start]    │  │
│   └─────────────────────────┘  │
│                                 │
│   💪 HYPERTROPHY                │
│   ┌─────────────────────────┐  │
│   │ High Frequency Full Body│  │
│   │ 8 weeks • 5x/week       │  │
│   │ Intermediate            │  │
│   │              [Start]    │  │
│   └─────────────────────────┘  │
│                                 │
│   🎯 SPECIALIZATION             │
│   ┌─────────────────────────┐  │
│   │ Arm Hypertrophy         │  │
│   │ 4 weeks • Add-on        │  │
│   │ All levels              │  │
│   │              [Start]    │  │
│   └─────────────────────────┘  │
│                                 │
│   [Browse All Programs]         │
└─────────────────────────────────┘
```

**Program Detail**:
```
┌─────────────────────────────────┐
│ ← Powerbuilding 2.0             │
│                                 │
│   6-Week Program                │
│   4 workouts/week               │
│   Intermediate-Advanced         │
│                                 │
│   Overview:                     │
│   Combines powerlifting and     │
│   bodybuilding for strength     │
│   and size. Focuses on Big 3    │
│   while adding hypertrophy      │
│   volume.                       │
│                                 │
│   What you'll need:             │
│   • Barbell + plates            │
│   • Squat rack                  │
│   • Bench                       │
│   • Dumbbells                   │
│   • Cable machine (optional)    │
│                                 │
│   Weekly Split:                 │
│   • Day 1: Squat Focus          │
│   • Day 2: Bench Focus          │
│   • Day 3: Deadlift Focus       │
│   • Day 4: Hypertrophy Accessory│
│                                 │
│   Expected Results:             │
│   • +10-20 lbs on Big 3         │
│   • +2-4 lbs bodyweight         │
│   • Improved work capacity      │
│                                 │
│   [Start Program]               │
│   [Preview Week 1]              │
└─────────────────────────────────┘
```

**Program Tracking**:
- Auto-progression (built into program)
- Deload weeks (programmed recovery)
- Progress checks (compare Week 1 vs Week 6)

---

### 2. Exercise Execution Guides

**In-Workout Reference**:
```
┌─────────────────────────────────┐
│   Bench Press                   │
│                                 │
│   [📹 Watch Technique Video]    │
│   (Jeff Nippard - 3:42)         │
│                                 │
│   ──────────────────────────    │
│   KEY CUES:                     │
│   • Retract scapulae            │
│   • Arch lower back             │
│   • Bar path: over nipples      │
│   • Elbows 45° angle            │
│   • Drive feet into ground      │
│                                 │
│   COMMON MISTAKES:              │
│   ❌ Flaring elbows (90°)       │
│   ❌ Bouncing off chest         │
│   ❌ Losing tightness           │
│                                 │
│   MUSCLES TARGETED:             │
│   Primary: Pectorals            │
│   Secondary: Anterior deltoids, │
│              Triceps            │
│                                 │
│   [Close]                       │
└─────────────────────────────────┘
```

**Library Access**:
- 200+ exercises with video guides
- Jeff Nippard's cues and science
- Accessible during workouts (quick reference)

---

### 3. Nutrition Protocols

**Meal Timing Optimization**:
```
┌─────────────────────────────────┐
│ ← Nutrition Timing              │
│                                 │
│   Based on your schedule:       │
│   Workout at 6:00 PM            │
│                                 │
│   🤖 AI Recommendations:        │
│                                 │
│   PRE-WORKOUT (4-5 PM)          │
│   • 30-40g carbs                │
│   • 20-30g protein              │
│   • Low fat                     │
│                                 │
│   Example:                      │
│   • Banana + Whey shake         │
│                                 │
│   POST-WORKOUT (7-8 PM)         │
│   • 40-60g carbs                │
│   • 30-40g protein              │
│   • Moderate fat                │
│                                 │
│   Example:                      │
│   • Chicken, rice, veggies      │
│                                 │
│   [Set Meal Reminders]          │
│   [Browse Recipes]              │
└─────────────────────────────────┘
```

**Recipe Library** (Jeff Nippard's Favorites):
```
┌─────────────────────────────────┐
│ ← Recipes                       │
│                                 │
│   🔍 [Search recipes...]        │
│                                 │
│   Filter by:                    │
│   ☐ High Protein                │
│   ☐ Low Calorie                 │
│   ☐ Vegetarian                  │
│   ☐ Quick (<15 min)             │
│                                 │
│   ──────────────────────────    │
│                                 │
│   🍳 Anabolic French Toast      │
│   520 cal • P: 45g F: 12g C: 55g│
│   [View Recipe]                 │
│                                 │
│   🥗 High-Protein Chicken Salad │
│   380 cal • P: 50g F: 8g C: 30g │
│   [View Recipe]                 │
│                                 │
│   🍌 Protein Oat Pancakes       │
│   450 cal • P: 35g F: 10g C: 60g│
│   [View Recipe]                 │
│                                 │
│   [Browse All Recipes]          │
└─────────────────────────────────┘
```

**Recipe Detail**:
```
┌─────────────────────────────────┐
│ ← Anabolic French Toast        │
│                                 │
│   [📷 Recipe Photo]             │
│                                 │
│   Macros (per serving):         │
│   520 cal • P: 45g F: 12g C: 55g│
│                                 │
│   Ingredients:                  │
│   • 4 slices whole wheat bread  │
│   • 3 whole eggs                │
│   • 1 scoop vanilla whey        │
│   • 1/2 cup egg whites          │
│   • Cinnamon, vanilla extract   │
│                                 │
│   Instructions:                 │
│   1. Mix eggs, whites, protein  │
│   2. Add cinnamon, vanilla      │
│   3. Dip bread, cook 3 min/side │
│   4. Top with sugar-free syrup  │
│                                 │
│   [Add to Meal Plan]            │
│   [Log This Meal]               │
└─────────────────────────────────┘
```

---

### 4. Educational Content

**Learn Tab**:
```
┌─────────────────────────────────┐
│   📚 Learn                      │
│                                 │
│   🎓 TRAINING SCIENCE           │
│   • Progressive overload        │
│   • Volume landmarks            │
│   • Frequency vs intensity      │
│   • Deload strategies           │
│                                 │
│   🍽️ NUTRITION SCIENCE          │
│   • Macro fundamentals          │
│   • Meal timing myths           │
│   • Adaptive TDEE explained     │
│   • Supplements guide           │
│                                 │
│   💪 EXERCISE LIBRARY           │
│   • Bench press variations      │
│   • Squat mechanics             │
│   • Deadlift setup              │
│   • Isolation techniques        │
│                                 │
│   [Browse All Topics]           │
└─────────────────────────────────┘
```

**Article Example**:
```
┌─────────────────────────────────┐
│ ← Progressive Overload          │
│                                 │
│   By Jeff Nippard               │
│   📖 5 min read                 │
│                                 │
│   Progressive overload is the   │
│   foundation of muscle growth.  │
│   Here's how to apply it:       │
│                                 │
│   1. ADD WEIGHT                 │
│   Most obvious method. Aim for  │
│   2.5-5 lbs per week on         │
│   compounds.                    │
│                                 │
│   2. ADD REPS                   │
│   If weight stalls, increase    │
│   reps within your target range │
│   (e.g., 8 → 12 reps).          │
│                                 │
│   3. ADD SETS                   │
│   Volume drives growth. Add 1-2 │
│   sets every 4-6 weeks.         │
│                                 │
│   [Continue Reading...]         │
│                                 │
│   Related:                      │
│   • Volume landmarks            │
│   • When to deload              │
└─────────────────────────────────┘
```

---

## Component Library

### Macro Ring (Dashboard Widget)

**Visual**:
```
     Calories
  ████████░░ 73%
  1,534 / 2,100

  P     F     C
 80%   64%   86%
128g  45g  180g
```

**Implementation Notes**:
- SVG circular progress
- Color-coded (Calories=blue, Protein=orange, Fat=yellow, Carbs=green)
- Animates on mount (0 → actual percentage)
- Tap to expand full nutrition view

---

### Rest Timer (Full-Screen Overlay)

**States**:
1. **Running**: Large countdown, progress bar
2. **10s Warning**: Color shift (blue → orange)
3. **Complete**: Haptic + sound, "Next Set" button

**Interactions**:
- Swipe down to dismiss (skip rest)
- Tap "+30s" to extend
- Background timer continues if app minimized

---

### Exercise Selector (Search + Browse)

**Search Mode**:
```
🔍 [bench press]

Results:
• Barbell Bench Press (Chest, primary)
• Incline Bench Press (Chest, upper)
• Dumbbell Bench Press (Chest, stabilizers)
```

**Browse Mode**:
```
Filter by Muscle:
[Chest ▼] [All Equipment ▼]

• Barbell Bench Press
• Incline Dumbbell Press
• Cable Flyes
• Push-ups
```

**Quick Actions**:
- Tap exercise → Add to workout
- Long-press → View technique guide

---

### Social Feed Card

**Anatomy**:
```
┌─────────────────────────────┐
│ @username  •  2h ago        │
│ ────────────────────────────│
│ [Content: workout/PR/photo] │
│                             │
│ 💬 8  👍 24  🔥 3            │
└─────────────────────────────┘
```

**Interactions**:
- Tap username → Profile
- Tap content → Detail view
- Tap 💬 → Comments
- Tap 👍 → Give kudos
- Long-press → Share, Report, Hide

---

## Interaction Patterns

### Swipe Gestures

**Feed**:
- Swipe left on card → Quick kudos
- Swipe right on card → Save post

**Workout Log**:
- Swipe left on exercise → Delete
- Swipe right on exercise → Duplicate

**Calendar View**:
- Swipe left/right → Previous/Next week

---

### Haptic Feedback

**When to Use**:
- Button press (light)
- Toggle switch (medium)
- PR achieved (heavy + success pattern)
- Rest timer complete (notification pattern)

**When NOT to Use**:
- Scrolling
- Text input
- Navigation

---

### Loading States

**Skeleton Screens**:
- Dashboard: Show placeholder rings, cards (no spinners)
- Feed: Show 3-4 placeholder cards
- Workout log: Show exercise list structure

**Optimistic Updates**:
- Logging set → Immediately show in UI, rollback on error
- Giving kudos → Increment count, revert on failure

---

## Data Models

### User Profile
```typescript
interface UserProfile {
  id: string
  username: string
  email: string
  created_at: Date

  // Physical
  age: number
  sex: 'male' | 'female' | 'other'
  height_cm: number
  current_weight_kg: number
  target_weight_kg: number

  // Goals
  primary_goal: 'lose_fat' | 'build_muscle' | 'maintain' | 'performance'
  activity_level: 1 | 2 | 3 | 4 | 5
  target_rate_kg_per_week: number

  // Calculated
  tdee_baseline: number
  tdee_adaptive: number
  calorie_target: number
  protein_target_g: number
  fat_target_g: number
  carb_target_g: number

  // Preferences
  units: 'metric' | 'imperial'
  theme: 'dark' | 'light' | 'auto'

  // Social
  followers_count: number
  following_count: number
  is_private: boolean
}
```

### Workout Session
```typescript
interface WorkoutSession {
  id: string
  user_id: string
  routine_id?: string
  started_at: Date
  completed_at?: Date
  duration_minutes: number
  total_volume_lbs: number
  rpe?: number  // 1-10 difficulty rating
  notes?: string
  is_public: boolean

  exercises: ExerciseSet[]
}

interface ExerciseSet {
  exercise_id: string
  set_number: number
  weight_lbs: number
  reps: number
  is_pr: boolean
  rpe?: number
}
```

### Nutrition Log
```typescript
interface NutritionDay {
  id: string
  user_id: string
  date: Date

  meals: Meal[]

  // Aggregated
  total_calories: number
  total_protein_g: number
  total_fat_g: number
  total_carbs_g: number
}

interface Meal {
  id: string
  meal_type: 'breakfast' | 'lunch' | 'dinner' | 'snack'
  logged_at: Date

  foods: FoodEntry[]
}

interface FoodEntry {
  food_id: string
  food_name: string
  serving_size: number
  serving_unit: string

  calories: number
  protein_g: number
  fat_g: number
  carbs_g: number
  fiber_g?: number

  source: 'usda' | 'custom' | 'ai_recognized'
}
```

### Social Activity
```typescript
interface FeedItem {
  id: string
  user_id: string
  type: 'workout' | 'pr' | 'progress_photo' | 'achievement'
  created_at: Date

  content: WorkoutPost | PRPost | PhotoPost | AchievementPost

  kudos_count: number
  comment_count: number
  is_kudoed_by_me: boolean
}

interface WorkoutPost {
  workout_id: string
  duration_minutes: number
  total_volume_lbs: number
  pr_count: number
}

interface PRPost {
  exercise_name: string
  weight_lbs: number
  reps: number
  estimated_1rm: number
}
```

---

## Privacy & Data Control

### Privacy Settings
```
┌─────────────────────────────────┐
│ ← Privacy                       │
│                                 │
│   WHO CAN SEE MY WORKOUTS       │
│   ● Everyone                    │
│   ○ Friends only                │
│   ○ Private (only me)           │
│                                 │
│   WHO CAN SEE MY NUTRITION      │
│   ○ Everyone                    │
│   ● Friends only                │
│   ○ Private (only me)           │
│                                 │
│   WHO CAN MESSAGE ME            │
│   ○ Everyone                    │
│   ● Friends only                │
│   ○ No one                      │
│                                 │
│   PROFILE VISIBILITY            │
│   ● Public profile              │
│   ○ Private profile             │
│                                 │
│   DATA SHARING                  │
│   ○ Share anonymized data for   │
│     research                    │
│                                 │
│   [Save Changes]                │
└─────────────────────────────────┘
```

### Data Export
```
Settings → Data → Export All Data
   ↓
Generate CSV/JSON package:
- workouts.csv
- nutrition.csv
- weight.csv
- profile.json
   ↓
Email download link
```

---

## Technical Architecture Summary

### Frontend
- **Framework**: React 18 + TypeScript
- **State**: Zustand (global), TanStack Query (server)
- **Routing**: React Router v6
- **Styling**: Tailwind CSS + Headless UI
- **Charts**: Recharts
- **Animation**: Framer Motion

### Backend
- **Language**: C++20
- **HTTP**: Crow framework
- **Database**: PostgreSQL 16
- **ORM**: libpqxx (raw SQL)
- **Auth**: JWT (HS256)
- **ML**: ONNX Runtime (food recognition)
- **LLM**: OpenAI API (GPT-4V) / Anthropic API (Claude)

### Infrastructure
- **Containerization**: Docker + Docker Compose
- **CI/CD**: GitHub Actions
- **Database Migrations**: Numbered SQL files (001-012)
- **Secrets Management**: Environment variables (.env)

---

## Implementation Priority

### Phase 1: Core (Weeks 1-4)
1. Authentication & User Profiles
2. Workout Logging
3. Nutrition Logging (search, quick add)
4. Dashboard (basic stats)

### Phase 2: Intelligence (Weeks 5-8)
1. AI Food Recognition
2. Adaptive TDEE
3. Weight Tracking & Trends
4. Calendar Integration (Google Calendar OAuth)
5. AI Workout Scheduling

### Phase 3: Social (Weeks 9-12)
1. Social Feed
2. Friends System
3. Kudos & Comments
4. Messaging (DMs)
5. Clubs & Challenges

### Phase 4: Jeff Nippard (Weeks 13-16)
1. Program Library
2. Exercise Guides (videos, cues)
3. Recipe Library
4. Educational Content
5. Meal Timing Recommendations

### Phase 5: Polish (Weeks 17-20)
1. Progress Photos
2. Advanced Analytics (volume, muscle distribution)
3. Gamification (badges, streaks)
4. Performance Optimization
5. Testing & Bug Fixes

---

## Success Metrics

### Engagement
- Daily Active Users (DAU)
- Workout logging rate (% of scheduled workouts logged)
- Nutrition logging consistency (days logged per week)
- Social interactions (kudos, comments per user per week)

### Retention
- D1, D7, D30 retention rates
- Weekly workout adherence (vs calendar schedule)
- Feature adoption (% using AI scheduling, food recognition)

### Quality
- Food recognition accuracy (measured via user corrections)
- TDEE prediction error (actual vs predicted weight change)
- App performance (load time < 2s, API latency < 200ms p95)

### Business (Future)
- Conversion rate (free → paid, if premium tier added)
- Referral rate (invites sent per user)
- NPS (Net Promoter Score)

---

## Open Questions for User

1. **Monetization Strategy**:
   - Free forever with ads?
   - Freemium (basic free, premium $9.99/mo)?
   - One-time purchase ($49.99)?
   - No monetization (personal project)?

2. **Platform Priority**:
   - iOS first, Android later?
   - Both simultaneously (React Native)?
   - Web app first (PWA), native later?

3. **Jeff Nippard Partnership**:
   - Official partnership (licensing content)?
   - Inspired by his methodology (no direct content)?
   - How to handle exercise videos (host ourselves, YouTube embeds)?

4. **AI Model Hosting**:
   - Food recognition: Local ONNX (privacy) vs cloud (accuracy)?
   - LLM costs: How many requests per user per month is acceptable?
   - Self-host LLM (Llama 3) vs API (OpenAI/Anthropic)?

5. **Calendar Permissions**:
   - Read-only calendar access (can't modify events)?
   - Allow FORGE to write workout events back to calendar?

6. **Social Features**:
   - Start with friends-only (controlled community)?
   - Public from day 1 (growth focus)?
   - Moderation strategy for user-generated content?

---

## Next Steps

**After Design Approval**:

1. **User Reviews This Document**
   - Mark sections for revision
   - Answer open questions
   - Approve overall direction

2. **Refine Design** (if needed)
   - Iterate on specific flows
   - Adjust features based on feedback

3. **Create Implementation Plan**
   - Break into PRs (1 feature per PR)
   - Estimate effort (t-shirt sizes: S/M/L/XL)
   - Prioritize ruthlessly

4. **Begin Development**
   - Phase 1, Week 1: Auth & User Profiles
   - PR-driven workflow
   - CodeRabbit reviews every PR
   - Continuous deployment

---

**End of Design Specification**

This document represents the complete vision for FORGE. Every screen, flow, AI integration, and social feature is described in detail. Once approved, we'll build this systematically, one PR at a time, with quality and user experience as the north star.

Ready to build? 🚀
