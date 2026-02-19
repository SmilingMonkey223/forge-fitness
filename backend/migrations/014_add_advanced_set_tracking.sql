-- Migration 014: Add Advanced Set Tracking
-- Add RIR, partial reps, failure sets, rest time, and superset support

-- Add new columns to exercise_sets table
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS rir INT CHECK (rir >= 0 AND rir <= 10);
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS partial_reps BOOLEAN DEFAULT FALSE;
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS taken_to_failure BOOLEAN DEFAULT FALSE;
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS rest_time_seconds INT CHECK (rest_time_seconds >= 0);
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS superset_with UUID REFERENCES workout_exercises(id) ON DELETE SET NULL;
ALTER TABLE exercise_sets ADD COLUMN IF NOT EXISTS notes TEXT;

-- Add column comments
COMMENT ON COLUMN exercise_sets.rir IS 'Reps In Reserve (0-10): How many more reps could be done before failure. 0 = absolute failure, 3 = could do 3 more reps';
COMMENT ON COLUMN exercise_sets.partial_reps IS 'Whether this set included partial reps (half reps, quarter reps, etc.)';
COMMENT ON COLUMN exercise_sets.taken_to_failure IS 'Whether this set was taken to absolute muscular failure';
COMMENT ON COLUMN exercise_sets.rest_time_seconds IS 'Actual rest time taken before this set (in seconds)';
COMMENT ON COLUMN exercise_sets.superset_with IS 'If this set is part of a superset, references the paired exercise';
COMMENT ON COLUMN exercise_sets.notes IS 'Per-set notes (e.g., "felt heavy", "good form", "used straps")';

-- Create index for superset queries
CREATE INDEX IF NOT EXISTS idx_exercise_sets_superset ON exercise_sets(superset_with) WHERE superset_with IS NOT NULL;

-- Add target RIR to workout_exercises (what the program prescribes)
ALTER TABLE workout_exercises ADD COLUMN IF NOT EXISTS target_rir INT CHECK (target_rir >= 0 AND target_rir <= 10);
ALTER TABLE workout_exercises ADD COLUMN IF NOT EXISTS target_rest_seconds INT DEFAULT 90 CHECK (target_rest_seconds >= 0);
ALTER TABLE workout_exercises ADD COLUMN IF NOT EXISTS is_superset BOOLEAN DEFAULT FALSE;

COMMENT ON COLUMN workout_exercises.target_rir IS 'Target RIR for this exercise (programmed difficulty)';
COMMENT ON COLUMN workout_exercises.target_rest_seconds IS 'Recommended rest time between sets for this exercise';
COMMENT ON COLUMN workout_exercises.is_superset IS 'Whether this exercise is part of a superset';

-- Add RPE (Rate of Perceived Exertion) to workouts for overall session difficulty
ALTER TABLE workouts ADD COLUMN IF NOT EXISTS rpe INT CHECK (rpe >= 1 AND rpe <= 10);
ALTER TABLE workouts ADD COLUMN IF NOT EXISTS notes TEXT;

COMMENT ON COLUMN workouts.rpe IS 'Rate of Perceived Exertion for entire workout (1-10). 1 = very easy, 10 = maximal effort';
COMMENT ON COLUMN workouts.notes IS 'Post-workout notes (how it felt, energy levels, etc.)';

-- Create view for easy PR detection
CREATE OR REPLACE VIEW exercise_personal_records AS
SELECT
  ws.user_id,
  we.exercise_id,
  e.name as exercise_name,
  es.weight_lbs,
  es.reps,
  -- Calculate estimated 1RM using Brzycki formula
  ROUND(es.weight_lbs * (36.0 / (37.0 - es.reps))::numeric, 1) as estimated_1rm,
  es.created_at as achieved_at,
  ws.id as workout_id,
  ROW_NUMBER() OVER (
    PARTITION BY ws.user_id, we.exercise_id, es.reps
    ORDER BY es.weight_lbs DESC, es.created_at DESC
  ) as is_pr
FROM exercise_sets es
JOIN workout_exercises we ON es.workout_exercise_id = we.id
JOIN workouts ws ON we.workout_id = ws.id
JOIN exercises e ON we.exercise_id = e.id
WHERE es.weight_lbs > 0 AND es.reps > 0;

COMMENT ON VIEW exercise_personal_records IS 'Shows personal records for each exercise by rep range. is_pr = 1 means current PR for that rep count';

-- Create view for volume tracking per muscle group
CREATE OR REPLACE VIEW muscle_group_volume AS
SELECT
  ws.user_id,
  ws.completed_at::date as workout_date,
  DATE_TRUNC('week', ws.completed_at) as workout_week,
  e.primary_muscle as muscle_group,
  SUM(es.weight_lbs * es.reps) as total_volume,
  COUNT(DISTINCT we.exercise_id) as exercise_count,
  SUM(es.reps) as total_reps
FROM exercise_sets es
JOIN workout_exercises we ON es.workout_exercise_id = we.id
JOIN workouts ws ON we.workout_id = ws.id
JOIN exercises e ON we.exercise_id = e.id
WHERE ws.completed_at IS NOT NULL
GROUP BY ws.user_id, ws.completed_at::date, DATE_TRUNC('week', ws.completed_at), e.primary_muscle;

COMMENT ON VIEW muscle_group_volume IS 'Aggregates training volume per muscle group by day and week';
