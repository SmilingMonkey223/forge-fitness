-- Migration 009: Progress Photos
-- Phase 2: Progress photo tracking for visual body composition changes

CREATE TABLE IF NOT EXISTS progress_photos (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    date DATE NOT NULL,
    angle VARCHAR(10) NOT NULL CHECK (angle IN ('front', 'side', 'back')),
    image_path TEXT NOT NULL,
    weight_kg FLOAT CHECK (weight_kg >= 30.0 AND weight_kg <= 300.0),
    notes TEXT CHECK (LENGTH(notes) <= 500),
    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_progress_photos_user_date ON progress_photos(user_id, date DESC);
CREATE INDEX idx_progress_photos_user_angle ON progress_photos(user_id, angle, date DESC);

-- Comments
COMMENT ON TABLE progress_photos IS 'Progress photos for visual tracking of body composition changes';
COMMENT ON COLUMN progress_photos.angle IS 'Photo angle: front, side, or back';
COMMENT ON COLUMN progress_photos.image_path IS 'Path to image file in object storage (MinIO)';
COMMENT ON COLUMN progress_photos.weight_kg IS 'Optional body weight at time of photo (from weight_entries if available)';
COMMENT ON COLUMN progress_photos.notes IS 'Optional user notes (max 500 chars)';
