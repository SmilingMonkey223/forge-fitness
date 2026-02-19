-- Migration 017: Create Social Feed System
-- Implements Strava-like social features: feed, kudos, comments, follows

-- User follows
CREATE TABLE follows (
  follower_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  following_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  approved BOOLEAN DEFAULT TRUE,
  created_at TIMESTAMP DEFAULT NOW(),
  PRIMARY KEY(follower_id, following_id),
  CHECK (follower_id != following_id)
);

-- Feed posts (workout activities, PRs, achievements, progress photos)
CREATE TABLE feed_posts (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  post_type VARCHAR(50) NOT NULL CHECK (post_type IN ('workout', 'pr', 'progress_photo', 'achievement', 'milestone')),

  -- References
  workout_id UUID REFERENCES workouts(id) ON DELETE CASCADE,

  -- Content
  title VARCHAR(200),
  description TEXT,
  image_url TEXT,

  -- Metadata
  visibility VARCHAR(20) DEFAULT 'everyone' CHECK (visibility IN ('everyone', 'followers', 'only_me')),

  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Kudos (like/upvote system, permanent and cannot be undone per Strava spec)
CREATE TABLE kudos (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  post_id UUID NOT NULL REFERENCES feed_posts(id) ON DELETE CASCADE,
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  created_at TIMESTAMP DEFAULT NOW(),
  UNIQUE(post_id, user_id)
);

-- Comments (flat structure, no threading per Strava spec)
CREATE TABLE comments (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  post_id UUID NOT NULL REFERENCES feed_posts(id) ON DELETE CASCADE,
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  content TEXT NOT NULL CHECK (LENGTH(content) >= 1 AND LENGTH(content) <= 1000),
  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- User profiles (extended from base users table)
CREATE TABLE user_profiles (
  user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  bio TEXT,
  avatar_url TEXT,
  location VARCHAR(100),

  -- Privacy settings
  profile_visibility VARCHAR(20) DEFAULT 'everyone' CHECK (profile_visibility IN ('everyone', 'followers', 'only_me')),
  message_privacy VARCHAR(20) DEFAULT 'mutuals' CHECK (message_privacy IN ('everyone', 'followers', 'mutuals', 'none')),

  -- Stats (cached for performance)
  follower_count INT DEFAULT 0,
  following_count INT DEFAULT 0,
  workout_count INT DEFAULT 0,
  total_volume_lifetime BIGINT DEFAULT 0,

  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Achievements (badges, milestones)
CREATE TABLE achievements (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  name VARCHAR(100) NOT NULL,
  description TEXT,
  icon VARCHAR(50),
  category VARCHAR(50) CHECK (category IN ('volume', 'consistency', 'strength', 'pr', 'social')),
  tier VARCHAR(20) CHECK (tier IN ('bronze', 'silver', 'gold', 'platinum')),
  requirement_type VARCHAR(50),
  requirement_value INT
);

-- User achievements (earned badges)
CREATE TABLE user_achievements (
  user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  achievement_id UUID NOT NULL REFERENCES achievements(id) ON DELETE CASCADE,
  earned_at TIMESTAMP DEFAULT NOW(),
  PRIMARY KEY(user_id, achievement_id)
);

-- Indexes for performance
CREATE INDEX idx_follows_follower ON follows(follower_id);
CREATE INDEX idx_follows_following ON follows(following_id);
CREATE INDEX idx_feed_posts_user ON feed_posts(user_id, created_at DESC);
CREATE INDEX idx_feed_posts_type ON feed_posts(post_type, created_at DESC);
CREATE INDEX idx_feed_posts_visibility ON feed_posts(visibility);
CREATE INDEX idx_kudos_post ON kudos(post_id);
CREATE INDEX idx_kudos_user ON kudos(user_id);
CREATE INDEX idx_comments_post ON comments(post_id, created_at ASC);
CREATE INDEX idx_user_profiles_visibility ON user_profiles(profile_visibility);

-- Triggers to maintain follower counts
CREATE OR REPLACE FUNCTION update_follower_counts()
RETURNS TRIGGER AS $$
BEGIN
  IF TG_OP = 'INSERT' THEN
    UPDATE user_profiles SET follower_count = follower_count + 1 WHERE user_id = NEW.following_id;
    UPDATE user_profiles SET following_count = following_count + 1 WHERE user_id = NEW.follower_id;
  ELSIF TG_OP = 'DELETE' THEN
    UPDATE user_profiles SET follower_count = GREATEST(0, follower_count - 1) WHERE user_id = OLD.following_id;
    UPDATE user_profiles SET following_count = GREATEST(0, following_count - 1) WHERE user_id = OLD.follower_id;
  END IF;
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_follower_counts_trigger
AFTER INSERT OR DELETE ON follows
FOR EACH ROW
EXECUTE FUNCTION update_follower_counts();

-- View for personalized feed (follows + own posts)
CREATE OR REPLACE VIEW personalized_feed AS
SELECT DISTINCT
  fp.*,
  u.username,
  u.email,
  up.avatar_url,
  (SELECT COUNT(*) FROM kudos k WHERE k.post_id = fp.id) as kudos_count,
  (SELECT COUNT(*) FROM comments c WHERE c.post_id = fp.id) as comment_count
FROM feed_posts fp
JOIN users u ON fp.user_id = u.id
LEFT JOIN user_profiles up ON u.id = up.user_id
WHERE fp.visibility != 'only_me'
ORDER BY fp.created_at DESC;

-- Seed default achievements
INSERT INTO achievements (name, description, icon, category, tier, requirement_type, requirement_value)
VALUES
  ('First Workout', 'Complete your first workout', '🎯', 'consistency', 'bronze', 'workout_count', 1),
  ('Week Warrior', 'Complete 7 workouts', '💪', 'consistency', 'silver', 'workout_count', 7),
  ('Month Master', 'Complete 30 workouts', '🔥', 'consistency', 'gold', 'workout_count', 30),
  ('Century Club', 'Complete 100 workouts', '💯', 'consistency', 'platinum', 'workout_count', 100),

  ('Ton Lifter', 'Lift 1 ton in a single workout', '🏋️', 'volume', 'bronze', 'session_volume', 1000),
  ('Volume King', 'Lift 10 tons in a single workout', '👑', 'volume', 'gold', 'session_volume', 10000),

  ('PR Setter', 'Set your first PR', '🎉', 'pr', 'bronze', 'pr_count', 1),
  ('PR Machine', 'Set 10 PRs', '⚡', 'pr', 'silver', 'pr_count', 10),
  ('PR Legend', 'Set 50 PRs', '🌟', 'pr', 'gold', 'pr_count', 50),

  ('Social Butterfly', 'Follow 10 users', '🦋', 'social', 'bronze', 'following_count', 10),
  ('Community Leader', 'Gain 50 followers', '👥', 'social', 'gold', 'follower_count', 50);

-- Comments
COMMENT ON TABLE follows IS 'User follow relationships (Strava-like)';
COMMENT ON TABLE feed_posts IS 'Social feed posts (workouts, PRs, achievements, photos)';
COMMENT ON TABLE kudos IS 'Kudos/likes on feed posts (permanent, cannot undo)';
COMMENT ON TABLE comments IS 'Comments on feed posts (flat structure, no threading)';
COMMENT ON TABLE user_profiles IS 'Extended user profile information and privacy settings';
COMMENT ON TABLE achievements IS 'Available achievements/badges';
COMMENT ON TABLE user_achievements IS 'Achievements earned by users';

COMMENT ON COLUMN kudos.id IS 'Kudos are permanent and cannot be undone (Strava behavior)';
COMMENT ON COLUMN feed_posts.visibility IS 'everyone = public, followers = followers only, only_me = private';
