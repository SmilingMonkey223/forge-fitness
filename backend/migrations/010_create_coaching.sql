-- Migration 010: Adaptive TDEE & Coaching
-- Phase 2: Weekly check-ins, adaptive TDEE calculation, and coaching recommendations

CREATE TABLE IF NOT EXISTS weekly_checkins (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    checkin_date DATE NOT NULL,
    week_start_date DATE NOT NULL,
    week_end_date DATE NOT NULL,

    -- Weight metrics
    starting_weight_kg FLOAT,
    ending_weight_kg FLOAT,
    weight_change_kg FLOAT,
    target_rate_kg_per_week FLOAT,

    -- TDEE metrics
    formula_tdee_kcal FLOAT,
    adaptive_tdee_kcal FLOAT,
    average_intake_kcal FLOAT,

    -- Targets
    previous_calorie_target FLOAT,
    new_calorie_target FLOAT,
    calorie_adjustment FLOAT,

    -- Macro targets
    new_protein_target_g FLOAT,
    new_carbs_target_g FLOAT,
    new_fat_target_g FLOAT,

    -- Tracking consistency
    days_weight_logged INT,
    days_nutrition_logged INT,

    -- User action
    status VARCHAR(20) NOT NULL DEFAULT 'pending' CHECK (status IN ('pending', 'accepted', 'skipped', 'overridden')),
    user_notes TEXT CHECK (LENGTH(user_notes) <= 1000),

    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),

    UNIQUE(user_id, week_start_date)
);

CREATE INDEX idx_weekly_checkins_user_date ON weekly_checkins(user_id, checkin_date DESC);

-- Comments
COMMENT ON TABLE weekly_checkins IS 'Weekly check-in records with TDEE calculation and target adjustments';
COMMENT ON COLUMN weekly_checkins.adaptive_tdee_kcal IS 'Calculated TDEE based on actual weight change and intake';
COMMENT ON COLUMN weekly_checkins.formula_tdee_kcal IS 'TDEE from Mifflin-St Jeor formula for comparison';
COMMENT ON COLUMN weekly_checkins.calorie_adjustment IS 'Recommended adjustment to calorie target (±50-100 kcal)';
COMMENT ON COLUMN weekly_checkins.status IS 'User response: accepted, skipped, or overridden with custom targets';
