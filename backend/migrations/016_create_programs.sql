-- Migration 016: Create Program Structure
-- Add AI coaching mode and tracker mode support

-- Program templates (6 pre-built + user-created)
CREATE TABLE programs (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  name VARCHAR(100) NOT NULL,
  description TEXT,
  author_id UUID REFERENCES users(id) ON DELETE SET NULL,
  goal VARCHAR(50) NOT NULL CHECK (goal IN ('strength', 'hypertrophy', 'mixed', 'endurance', 'powerlifting', 'bodybuilding')),
  experience_level VARCHAR(50) NOT NULL CHECK (experience_level IN ('beginner', 'intermediate', 'advanced')),
  duration_weeks INT NOT NULL CHECK (duration_weeks >= 1 AND duration_weeks <= 52),
  workouts_per_week INT NOT NULL CHECK (workouts_per_week >= 1 AND workouts_per_week <= 7),
  is_official BOOLEAN DEFAULT FALSE,
  is_public BOOLEAN DEFAULT TRUE,
  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Program weeks (specific configuration for each week)
CREATE TABLE program_weeks (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  program_id UUID NOT NULL REFERENCES programs(id) ON DELETE CASCADE,
  week_number INT NOT NULL CHECK (week_number >= 1),
  deload_week BOOLEAN DEFAULT FALSE,
  intensity_multiplier DECIMAL(3,2) DEFAULT 1.0 CHECK (intensity_multiplier >= 0.5 AND intensity_multiplier <= 1.5),
  volume_multiplier DECIMAL(3,2) DEFAULT 1.0 CHECK (volume_multiplier >= 0.5 AND volume_multiplier <= 2.0),
  notes TEXT,
  UNIQUE(program_id, week_number)
);

-- Program workouts (template for each workout in the program)
CREATE TABLE program_workouts (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  program_id UUID NOT NULL REFERENCES programs(id) ON DELETE CASCADE,
  week_number INT NOT NULL,
  day_number INT NOT NULL CHECK (day_number >= 1 AND day_number <= 7),
  name VARCHAR(100) NOT NULL,
  description TEXT,
  estimated_duration_minutes INT,
  UNIQUE(program_id, week_number, day_number)
);

-- Program exercises (template for exercises in each workout)
CREATE TABLE program_exercises (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  program_workout_id UUID NOT NULL REFERENCES program_workouts(id) ON DELETE CASCADE,
  exercise_id UUID NOT NULL REFERENCES exercises(id) ON DELETE CASCADE,
  exercise_order INT NOT NULL,
  target_sets INT NOT NULL CHECK (target_sets >= 1 AND target_sets <= 10),
  target_reps_min INT CHECK (target_reps_min >= 1),
  target_reps_max INT CHECK (target_reps_max >= target_reps_min),
  target_rir INT CHECK (target_rir >= 0 AND target_rir <= 10),
  target_rest_seconds INT DEFAULT 90 CHECK (target_rest_seconds >= 0),
  is_superset BOOLEAN DEFAULT FALSE,
  superset_group INT,
  progression_scheme VARCHAR(50) DEFAULT 'linear' CHECK (progression_scheme IN ('linear', 'undulated', 'wave', 'step')),
  notes TEXT,
  UNIQUE(program_workout_id, exercise_order)
);

-- AI coaching state (tracks user's current program and progress)
CREATE TABLE user_coaching_state (
  user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  mode VARCHAR(20) NOT NULL DEFAULT 'tracker' CHECK (mode IN ('ai_coaching', 'tracker')),
  current_program_id UUID REFERENCES programs(id) ON DELETE SET NULL,
  current_week INT DEFAULT 1,
  program_start_date DATE,
  last_check_in DATE,
  tdee_adaptive DECIMAL(6,1),
  energy_balance VARCHAR(20) CHECK (energy_balance IN ('aggressive_cut', 'moderate_cut', 'maintenance', 'lean_bulk', 'aggressive_bulk')),
  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Weekly check-ins (for AI coaching adjustments)
CREATE TABLE weekly_checkins (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  week_start DATE NOT NULL,
  week_end DATE NOT NULL,

  -- Subjective metrics
  hunger_rating INT CHECK (hunger_rating >= 1 AND hunger_rating <= 5),
  energy_rating INT CHECK (energy_rating >= 1 AND energy_rating <= 5),
  recovery_rating INT CHECK (recovery_rating >= 1 AND recovery_rating <= 5),
  sleep_quality_rating INT CHECK (sleep_quality_rating >= 1 AND sleep_quality <= 5),

  -- Objective metrics
  average_weight_lbs DECIMAL(5,1),
  weight_change_lbs DECIMAL(4,1),
  total_workouts_completed INT DEFAULT 0,
  total_sets_completed INT DEFAULT 0,

  -- AI recommendations
  ai_recommendation TEXT,
  adjustment_made VARCHAR(100),

  notes TEXT,
  created_at TIMESTAMP DEFAULT NOW(),
  UNIQUE(user_id, week_start)
);

-- AI coaching questionnaire responses (initial setup)
CREATE TABLE coaching_questionnaire (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,

  -- Training history
  experience_level VARCHAR(50) NOT NULL,
  years_training DECIMAL(3,1),
  current_frequency INT CHECK (current_frequency >= 0 AND current_frequency <= 7),

  -- Goals
  primary_goal VARCHAR(50) NOT NULL,
  secondary_goal VARCHAR(50),
  goal_timeframe_weeks INT,

  -- Constraints
  available_days_per_week INT NOT NULL CHECK (available_days_per_week >= 1 AND available_days_per_week <= 7),
  session_duration_minutes INT CHECK (session_duration_minutes >= 20 AND session_duration_minutes <= 180),
  available_equipment JSONB DEFAULT '[]'::jsonb,

  -- Injuries/limitations
  injuries_or_limitations TEXT,
  exercises_to_avoid JSONB DEFAULT '[]'::jsonb,

  -- Preferences
  preferred_rep_range VARCHAR(50),
  preferred_intensity VARCHAR(50),

  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Seed official FORGE programs
INSERT INTO programs (name, description, goal, experience_level, duration_weeks, workouts_per_week, is_official)
VALUES
  ('FORGE Beginner Full Body', 'Perfect starting point for new lifters. 3x/week full body training focusing on compound movements and building a foundation.', 'mixed', 'beginner', 12, 3, TRUE),
  ('FORGE Hypertrophy Push/Pull/Legs', 'Classic 6-day PPL split optimized for muscle growth. High volume with progressive overload.', 'hypertrophy', 'intermediate', 8, 6, TRUE),
  ('FORGE Strength 5/3/1', 'Jim Wendler''s proven 5/3/1 methodology adapted for FORGE. 4 days/week focusing on main lifts.', 'strength', 'intermediate', 12, 4, TRUE),
  ('FORGE Powerlifting Peaking', 'Advanced 12-week peaking program for competition prep. Focuses on squat, bench, deadlift.', 'powerlifting', 'advanced', 12, 4, TRUE),
  ('FORGE Upper/Lower Split', 'Balanced 4-day upper/lower split for intermediate lifters. Great for strength and size.', 'mixed', 'intermediate', 8, 4, TRUE),
  ('FORGE Bodybuilding Bro Split', 'Classic 5-day bodybuilding split: Chest, Back, Shoulders, Arms, Legs. High volume per muscle group.', 'bodybuilding', 'advanced', 8, 5, TRUE);

-- Indexes for performance
CREATE INDEX idx_programs_author ON programs(author_id) WHERE author_id IS NOT NULL;
CREATE INDEX idx_programs_official ON programs(is_official) WHERE is_official = TRUE;
CREATE INDEX idx_program_weeks_program ON program_weeks(program_id, week_number);
CREATE INDEX idx_program_workouts_program ON program_workouts(program_id, week_number, day_number);
CREATE INDEX idx_program_exercises_workout ON program_exercises(program_workout_id, exercise_order);
CREATE INDEX idx_weekly_checkins_user ON weekly_checkins(user_id, week_start DESC);
CREATE INDEX idx_coaching_state_program ON user_coaching_state(current_program_id) WHERE current_program_id IS NOT NULL;

-- Comments
COMMENT ON TABLE programs IS 'Program templates (official FORGE programs + user-created)';
COMMENT ON TABLE program_weeks IS 'Week-by-week configuration for periodization';
COMMENT ON TABLE program_workouts IS 'Template for each workout in the program';
COMMENT ON TABLE program_exercises IS 'Exercises within each workout template';
COMMENT ON TABLE user_coaching_state IS 'Tracks user''s current program and AI coaching state';
COMMENT ON TABLE weekly_checkins IS 'Weekly check-ins for AI coaching adjustments';
COMMENT ON TABLE coaching_questionnaire IS 'Initial AI coaching setup questionnaire responses';

COMMENT ON COLUMN program_exercises.progression_scheme IS 'How to progress this exercise: linear (add weight), undulated (vary intensity), wave (cycles), step (jumps)';
COMMENT ON COLUMN weekly_checkins.hunger_rating IS '1 = Starving, 2 = Very hungry, 3 = Normal, 4 = Satisfied, 5 = Overfull';
COMMENT ON COLUMN weekly_checkins.energy_rating IS '1 = Exhausted, 2 = Low energy, 3 = Normal, 4 = Good energy, 5 = Great energy';
COMMENT ON COLUMN weekly_checkins.recovery_rating IS '1 = Very sore, 2 = Quite sore, 3 = Normal soreness, 4 = Well recovered, 5 = Fully recovered';
