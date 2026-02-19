-- Migration 013: Create Exercise Videos Table
-- Stores curated YouTube video links for each exercise

CREATE TABLE exercise_videos (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  exercise_id UUID NOT NULL REFERENCES exercises(id) ON DELETE CASCADE,

  -- Video metadata
  video_type VARCHAR(50) NOT NULL, -- 'primary', 'alternative', 'mistakes', 'advanced'
  youtube_video_id VARCHAR(20) NOT NULL, -- e.g., 'dQw4w9WgXcQ'
  youtube_url TEXT NOT NULL, -- Full URL for fallback

  -- Content details
  title VARCHAR(200) NOT NULL,
  channel_name VARCHAR(100) NOT NULL,
  channel_id VARCHAR(50),
  duration_seconds INT,
  view_count BIGINT,
  upload_date DATE,

  -- Curated timestamps (JSON)
  -- Example: {"setup": 0, "execution": 90, "common_mistakes": 180, "variations": 300}
  timestamps JSONB DEFAULT '{}'::jsonb,

  -- Quality metrics
  is_verified BOOLEAN DEFAULT FALSE, -- Manually verified by FORGE team
  quality_score INT DEFAULT 5 CHECK (quality_score >= 1 AND quality_score <= 10),

  -- Usage tracking
  view_count_forge INT DEFAULT 0, -- How many FORGE users watched
  last_checked TIMESTAMP, -- Last time we verified video still exists

  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Indexes for performance
CREATE INDEX idx_exercise_videos_exercise_id ON exercise_videos(exercise_id);
CREATE INDEX idx_exercise_videos_type ON exercise_videos(video_type);
CREATE INDEX idx_exercise_videos_quality ON exercise_videos(quality_score DESC);

-- Constraint: Each exercise should have exactly 1 primary video
CREATE UNIQUE INDEX idx_one_primary_per_exercise
  ON exercise_videos(exercise_id)
  WHERE video_type = 'primary';

-- Table for offline video caching
CREATE TABLE user_video_cache (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  video_id UUID NOT NULL REFERENCES exercise_videos(id) ON DELETE CASCADE,

  -- Cached video data
  cache_url TEXT, -- Local file path or blob storage URL
  cache_size_bytes BIGINT,
  cache_quality VARCHAR(20), -- '360p', '720p', '1080p'

  -- Metadata
  downloaded_at TIMESTAMP DEFAULT NOW(),
  last_accessed TIMESTAMP DEFAULT NOW(),
  expires_at TIMESTAMP DEFAULT NOW() + INTERVAL '30 days', -- Auto-delete after 30 days unused

  UNIQUE(user_id, video_id)
);

-- Index for cache cleanup cron job
CREATE INDEX idx_user_video_cache_expires ON user_video_cache(expires_at);

-- Seed with a few example videos for common exercises
INSERT INTO exercise_videos (exercise_id, video_type, youtube_video_id, youtube_url, title, channel_name, quality_score, is_verified, timestamps) VALUES
  -- Bench Press (find exercise_id from exercises table)
  (
    (SELECT id FROM exercises WHERE name = 'Bench Press' LIMIT 1),
    'primary',
    'vthMCtgVtFw',
    'https://www.youtube.com/watch?v=vthMCtgVtFw',
    'How to Bench Press with Proper Form',
    'Jeff Nippard',
    10,
    TRUE,
    '{"setup": 0, "execution": 90, "common_mistakes": 240}'::jsonb
  ),
  (
    (SELECT id FROM exercises WHERE name = 'Squat' LIMIT 1),
    'primary',
    'vmNPOjaGrVE',
    'https://www.youtube.com/watch?v=vmNPOjaGrVE',
    'How To Squat Properly',
    'Alan Thrall',
    10,
    TRUE,
    '{"setup": 0, "execution": 60, "common_mistakes": 180}'::jsonb
  ),
  (
    (SELECT id FROM exercises WHERE name = 'Deadlift' LIMIT 1),
    'primary',
    'wYREQkVtvEc',
    'https://www.youtube.com/watch?v=wYREQkVtvEc',
    'How to Deadlift with Proper Form',
    'Jeff Nippard',
    10,
    TRUE,
    '{"setup": 0, "execution": 120, "common_mistakes": 240}'::jsonb
  )
ON CONFLICT DO NOTHING;

COMMENT ON TABLE exercise_videos IS 'Curated YouTube video links for exercise demonstrations';
COMMENT ON COLUMN exercise_videos.video_type IS 'primary: main technique video, alternative: different coach/angle, mistakes: what not to do, advanced: variations';
COMMENT ON COLUMN exercise_videos.timestamps IS 'JSON object with timestamp markers for quick jumps: setup, execution, common_mistakes, variations';
