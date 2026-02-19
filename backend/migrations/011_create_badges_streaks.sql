-- Migration 011: Achievement Badges & Streaks
-- Phase 2: Gamification system for user engagement

CREATE TABLE IF NOT EXISTS badges (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT NOT NULL,
    category VARCHAR(20) NOT NULL CHECK (category IN ('workout', 'nutrition', 'consistency', 'milestone')),
    icon VARCHAR(50),
    tier VARCHAR(20) NOT NULL DEFAULT 'bronze' CHECK (tier IN ('bronze', 'silver', 'gold', 'platinum')),
    criteria_json JSONB NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_badges_category ON badges(category);

-- User badges (earned achievements)
CREATE TABLE IF NOT EXISTS user_badges (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    badge_id UUID NOT NULL REFERENCES badges(id) ON DELETE CASCADE,
    earned_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    progress_data JSONB,

    UNIQUE(user_id, badge_id)
);

CREATE INDEX idx_user_badges_user ON user_badges(user_id, earned_at DESC);
CREATE INDEX idx_user_badges_badge ON user_badges(badge_id);

-- Streaks tracking
CREATE TABLE IF NOT EXISTS user_streaks (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID UNIQUE NOT NULL REFERENCES users(id) ON DELETE CASCADE,

    -- Workout streaks
    current_workout_streak INT NOT NULL DEFAULT 0,
    best_workout_streak INT NOT NULL DEFAULT 0,
    last_workout_date DATE,

    -- Nutrition logging streaks
    current_logging_streak INT NOT NULL DEFAULT 0,
    best_logging_streak INT NOT NULL DEFAULT 0,
    last_logging_date DATE,

    -- Weight tracking streaks
    current_weight_streak INT NOT NULL DEFAULT 0,
    best_weight_streak INT NOT NULL DEFAULT 0,
    last_weight_date DATE,

    -- Streak freezes (earned by maintaining 7+ day streaks)
    workout_freezes_available INT NOT NULL DEFAULT 0,
    logging_freezes_available INT NOT NULL DEFAULT 0,
    weight_freezes_available INT NOT NULL DEFAULT 0,

    updated_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_user_streaks_user ON user_streaks(user_id);

-- Seed initial badges
INSERT INTO badges (code, name, description, category, icon, tier, criteria_json) VALUES
('first_workout', 'First Workout', 'Complete your first workout', 'milestone', 'dumbbell', 'bronze', '{"workouts_completed": 1}'),
('consistent_7', 'Consistent', '7-day workout streak', 'consistency', 'calendar', 'silver', '{"streak_days": 7}'),
('dedicated_30', 'Dedicated', '30-day workout streak', 'consistency', 'calendar', 'gold', '{"streak_days": 30}'),
('century_workouts', 'Century', 'Log 100 workouts', 'milestone', '100', 'platinum', '{"workouts_completed": 100}'),
('pr_machine', 'PR Machine', 'Hit 10 personal records', 'workout', 'trophy', 'gold', '{"prs_hit": 10}'),
('macro_master', 'Macro Master', 'Hit calorie target ±10% for 7 consecutive days', 'nutrition', 'target', 'gold', '{"streak_days": 7, "tolerance_percent": 10}'),
('protein_pro', 'Protein Pro', 'Hit protein target for 14 consecutive days', 'nutrition', 'protein', 'gold', '{"streak_days": 14}'),
('ai_logger', 'AI Logger', 'Log 50 meals using food recognition', 'nutrition', 'camera', 'silver', '{"ai_meals_logged": 50}'),
('weight_watcher', 'Weight Watcher', 'Log weight 30 days in a row', 'consistency', 'scale', 'gold', '{"streak_days": 30}'),
('body_documenter', 'Body Documenter', 'Take progress photos 4 weeks in a row', 'consistency', 'camera', 'silver', '{"weeks_consecutive": 4}')
ON CONFLICT (code) DO NOTHING;

-- Comments
COMMENT ON TABLE badges IS 'Available achievement badges in the system';
COMMENT ON TABLE user_badges IS 'Badges earned by users';
COMMENT ON TABLE user_streaks IS 'Tracking of user streaks for various activities';
COMMENT ON COLUMN user_streaks.workout_freezes_available IS 'Number of streak freezes available (earned by 7+ day streaks)';
