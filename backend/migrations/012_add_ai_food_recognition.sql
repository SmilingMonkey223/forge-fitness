-- Migration 012: AI Food Recognition
-- Phase 2: Add AI recognition fields to nutrition tracking

ALTER TABLE nutrition_log
ADD COLUMN IF NOT EXISTS source VARCHAR(20) DEFAULT 'manual' CHECK (source IN ('manual', 'ai', 'recent', 'custom')),
ADD COLUMN IF NOT EXISTS confidence_score FLOAT CHECK (confidence_score >= 0.0 AND confidence_score <= 1.0),
ADD COLUMN IF NOT EXISTS ai_tier INT CHECK (ai_tier IN (1, 2)),
ADD COLUMN IF NOT EXISTS image_path TEXT;

CREATE INDEX idx_nutrition_log_source ON nutrition_log(source);
CREATE INDEX idx_nutrition_log_ai_tier ON nutrition_log(ai_tier) WHERE ai_tier IS NOT NULL;

-- Create table for AI recognition tasks (async processing)
CREATE TABLE IF NOT EXISTS food_recognition_tasks (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    status VARCHAR(20) NOT NULL DEFAULT 'pending' CHECK (status IN ('pending', 'processing', 'completed', 'failed')),
    image_path TEXT NOT NULL,

    -- Tier 1 results (ONNX inference)
    tier1_predictions JSONB,
    tier1_confidence FLOAT,
    tier1_time_ms INT,

    -- Tier 2 results (LLM refinement)
    tier2_required BOOLEAN DEFAULT FALSE,
    tier2_results JSONB,
    tier2_time_ms INT,

    -- Final results
    recognized_items JSONB,
    error_message TEXT,

    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    completed_at TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_food_recognition_tasks_user ON food_recognition_tasks(user_id, created_at DESC);
CREATE INDEX idx_food_recognition_tasks_status ON food_recognition_tasks(status);

-- Comments
COMMENT ON COLUMN nutrition_log.source IS 'Source of nutrition entry: manual, AI recognition, recent foods, or custom food';
COMMENT ON COLUMN nutrition_log.confidence_score IS 'AI confidence score (0.0-1.0) when source is "ai"';
COMMENT ON COLUMN nutrition_log.ai_tier IS 'Which AI tier was used: 1 (ONNX) or 2 (LLM)';
COMMENT ON COLUMN nutrition_log.image_path IS 'Path to food photo if logged via AI recognition';

COMMENT ON TABLE food_recognition_tasks IS 'Async food recognition tasks for tracking and debugging';
COMMENT ON COLUMN food_recognition_tasks.tier1_predictions IS 'Top-5 predictions from ONNX model with confidence scores';
COMMENT ON COLUMN food_recognition_tasks.tier2_results IS 'Detailed results from LLM including portion estimates';
