-- Migration 008: Weight Tracking & Trend Analysis
-- Phase 2: Body weight tracking with trend smoothing

CREATE TABLE IF NOT EXISTS weight_entries (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    date DATE NOT NULL,
    weight_kg FLOAT NOT NULL CHECK (weight_kg >= 30.0 AND weight_kg <= 300.0),
    source VARCHAR(20) NOT NULL DEFAULT 'manual' CHECK (source IN ('manual', 'apple_health', 'google_fit')),
    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),

    -- One entry per user per day
    UNIQUE(user_id, date)
);

CREATE INDEX idx_weight_entries_user_date ON weight_entries(user_id, date DESC);

-- Comments
COMMENT ON TABLE weight_entries IS 'Daily body weight measurements for trend analysis and TDEE calculation';
COMMENT ON COLUMN weight_entries.weight_kg IS 'Body weight in kilograms (30-300 kg range)';
COMMENT ON COLUMN weight_entries.source IS 'Source of weight data: manual entry or sync from health app';
COMMENT ON COLUMN weight_entries.date IS 'Date of weigh-in (unique per user per day)';
