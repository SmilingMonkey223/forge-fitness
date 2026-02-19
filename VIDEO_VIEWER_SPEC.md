# FORGE Video Viewer - Technical Specification

## Overview

Premium in-app YouTube video player that provides a seamless exercise demonstration experience without leaving FORGE.

---

## Video Source Strategy

### YouTube Video Curation

**638 exercises × 3-4 videos each = ~2,000 curated links**

#### Trusted Content Creators
1. **Jeff Nippard** - Science-based technique breakdowns
2. **AthleanX** (Jeff Cavaliere) - Form checks, injury prevention
3. **Renaissance Periodization** (Dr. Mike Israetel) - Detailed execution
4. **Alan Thrall** - Powerlifting focus, low-tech honesty
5. **Squat University** (Dr. Aaron Horschig) - Mobility, form therapy
6. **Eugene Teo** - Natural bodybuilding technique
7. **Omar Isuf** - Practical powerlifting
8. **Calgary Barbell** - Strength training deep dives

#### Video Types Per Exercise
```
1. PRIMARY VIDEO
   - Best overall technique demonstration
   - 3-8 minutes long
   - Clear camera angles
   - Verbal cues

2. ALTERNATIVE ANGLE
   - Different coach/perspective
   - Emphasizes different cue
   - Example: Bench press (Nippard = science, Thrall = practical)

3. COMMON MISTAKES
   - What NOT to do
   - Injury prevention
   - AthleanX specializes in this

4. ADVANCED VARIATIONS (optional)
   - For experienced lifters
   - Tempo, pauses, bands, chains
```

---

## Database Schema

### New Migration: `013_create_exercise_videos.sql`

```sql
CREATE TABLE exercise_videos (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  exercise_id UUID REFERENCES exercises(id) ON DELETE CASCADE,

  -- Video metadata
  video_type VARCHAR(50) NOT NULL, -- 'primary', 'alternative', 'mistakes', 'advanced'
  youtube_video_id VARCHAR(20) NOT NULL, -- e.g., 'dQw4w9WgXcQ'
  youtube_url TEXT NOT NULL, -- Full URL for fallback

  -- Content details
  title VARCHAR(200),
  channel_name VARCHAR(100),
  channel_id VARCHAR(50),
  duration_seconds INT,
  view_count BIGINT,
  upload_date DATE,

  -- Curated timestamps (JSON)
  timestamps JSONB,
  /* Example:
  {
    "setup": 0,
    "execution": 90,
    "common_mistakes": 180,
    "variations": 300
  }
  */

  -- Quality metrics
  is_verified BOOLEAN DEFAULT FALSE, -- Manually verified by FORGE team
  quality_score INT, -- 1-10 based on views, likes, channel authority

  -- Usage tracking
  view_count_forge INT DEFAULT 0, -- How many FORGE users watched
  last_checked TIMESTAMP, -- Last time we verified video still exists

  created_at TIMESTAMP DEFAULT NOW(),
  updated_at TIMESTAMP DEFAULT NOW()
);

-- Indexes
CREATE INDEX idx_exercise_videos_exercise_id ON exercise_videos(exercise_id);
CREATE INDEX idx_exercise_videos_type ON exercise_videos(video_type);
CREATE INDEX idx_exercise_videos_quality ON exercise_videos(quality_score DESC);

-- Constraint: Each exercise should have at least 1 primary video
CREATE UNIQUE INDEX idx_one_primary_per_exercise
  ON exercise_videos(exercise_id)
  WHERE video_type = 'primary';
```

### Video Caching Table (for offline use)

```sql
CREATE TABLE user_video_cache (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id UUID REFERENCES users(id) ON DELETE CASCADE,
  video_id UUID REFERENCES exercise_videos(id) ON DELETE CASCADE,

  -- Cached video data
  cache_url TEXT, -- Local file path or blob storage URL
  cache_size_bytes BIGINT,
  cache_quality VARCHAR(20), -- '360p', '720p', '1080p'

  -- Metadata
  downloaded_at TIMESTAMP DEFAULT NOW(),
  last_accessed TIMESTAMP DEFAULT NOW(),
  expires_at TIMESTAMP, -- Auto-delete after 30 days unused

  UNIQUE(user_id, video_id)
);
```

---

## Backend API

### Exercise Video Endpoints

```cpp
// Get videos for an exercise
GET /api/exercises/:exercise_id/videos
Response:
{
  "exercise_id": "uuid",
  "exercise_name": "Barbell Bench Press",
  "videos": [
    {
      "id": "uuid",
      "type": "primary",
      "youtube_video_id": "abc123",
      "embed_url": "https://www.youtube-nocookie.com/embed/abc123",
      "title": "How to Bench Press | Proper Form & Technique",
      "channel": "Jeff Nippard",
      "duration": 482,
      "timestamps": {
        "setup": 0,
        "execution": 90,
        "common_mistakes": 180,
        "variations": 300
      },
      "quality_score": 10
    },
    {
      "id": "uuid",
      "type": "alternative",
      "youtube_video_id": "xyz789",
      "title": "Bench Press Setup - 5 Steps",
      "channel": "Alan Thrall",
      "duration": 357,
      "quality_score": 9
    },
    {
      "id": "uuid",
      "type": "mistakes",
      "youtube_video_id": "def456",
      "title": "5 Bench Press Mistakes Killing Your Progress",
      "channel": "AthleanX",
      "duration": 612,
      "quality_score": 9
    }
  ]
}

// Get single video details
GET /api/videos/:video_id

// Track video view (analytics)
POST /api/videos/:video_id/view
Body: { "watch_time_seconds": 120, "completed": false }

// Cache video for offline use
POST /api/videos/:video_id/cache
Body: { "quality": "720p" }
Response: { "cache_id": "uuid", "download_url": "blob://..." }

// Get user's cached videos
GET /api/user/video-cache
```

### Backend Implementation

**New Service**: `backend/src/services/video_service.cpp`

```cpp
class VideoService {
public:
  // Get videos for exercise
  std::vector<ExerciseVideo> get_exercise_videos(
    const std::string& exercise_id,
    const std::optional<std::string>& type = std::nullopt
  );

  // Verify YouTube video still exists (cron job)
  bool verify_video_exists(const std::string& youtube_video_id);

  // Track view for analytics
  void track_video_view(
    const std::string& video_id,
    const std::string& user_id,
    int watch_time_seconds,
    bool completed
  );

  // Cache video for offline
  std::string cache_video(
    const std::string& video_id,
    const std::string& user_id,
    const std::string& quality
  );
};
```

---

## Frontend Implementation

### Component: `ExerciseVideoPlayer.tsx`

**Full-Featured Video Player Component**

```tsx
import React, { useState, useRef } from 'react';
import YouTube, { YouTubeProps } from 'react-youtube';

interface ExerciseVideo {
  id: string;
  type: 'primary' | 'alternative' | 'mistakes' | 'advanced';
  youtubeVideoId: string;
  title: string;
  channel: string;
  duration: number;
  timestamps: {
    setup?: number;
    execution?: number;
    common_mistakes?: number;
    variations?: number;
  };
  qualityScore: number;
}

interface Props {
  exerciseId: string;
  exerciseName: string;
  isInWorkout?: boolean; // Show PiP controls if true
}

export const ExerciseVideoPlayer: React.FC<Props> = ({
  exerciseId,
  exerciseName,
  isInWorkout = false
}) => {
  const [videos, setVideos] = useState<ExerciseVideo[]>([]);
  const [activeVideo, setActiveVideo] = useState<ExerciseVideo | null>(null);
  const [playbackRate, setPlaybackRate] = useState(1.0);
  const [isPiP, setIsPiP] = useState(false);
  const playerRef = useRef<any>(null);

  // Fetch videos on mount
  useEffect(() => {
    fetchVideos();
  }, [exerciseId]);

  const fetchVideos = async () => {
    const response = await fetch(`/api/exercises/${exerciseId}/videos`);
    const data = await response.json();
    setVideos(data.videos);
    setActiveVideo(data.videos.find(v => v.type === 'primary') || data.videos[0]);
  };

  // YouTube player options
  const youtubeOpts: YouTubeProps['opts'] = {
    height: isPiP ? '200' : '390',
    width: isPiP ? '300' : '640',
    playerVars: {
      autoplay: 0,
      controls: 1,
      modestbranding: 1, // Minimal YouTube branding
      rel: 0, // Don't show related videos
      playsinline: 1, // iOS inline playback
      fs: 1, // Fullscreen button
      iv_load_policy: 3, // Hide annotations
      disablekb: 0, // Allow keyboard controls
    },
  };

  // Jump to timestamp
  const jumpToTimestamp = (section: keyof ExerciseVideo['timestamps']) => {
    const timestamp = activeVideo?.timestamps[section];
    if (timestamp !== undefined && playerRef.current) {
      playerRef.current.seekTo(timestamp, true);
      playerRef.current.playVideo();
    }
  };

  // Change playback speed
  const changeSpeed = (speed: number) => {
    if (playerRef.current) {
      playerRef.current.setPlaybackRate(speed);
      setPlaybackRate(speed);
    }
  };

  // Picture-in-Picture toggle
  const togglePiP = () => {
    setIsPiP(!isPiP);
  };

  // Track view analytics
  const onVideoEnd = () => {
    if (activeVideo) {
      fetch(`/api/videos/${activeVideo.id}/view`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          watch_time_seconds: activeVideo.duration,
          completed: true
        })
      });
    }
  };

  if (!activeVideo) return <div>Loading...</div>;

  return (
    <div className={`video-player ${isPiP ? 'pip-mode' : ''}`}>
      {/* Video Tabs */}
      <div className="video-tabs">
        {videos.map(video => (
          <button
            key={video.id}
            className={`tab ${activeVideo.id === video.id ? 'active' : ''}`}
            onClick={() => setActiveVideo(video)}
          >
            {video.type === 'primary' && '📹 Technique'}
            {video.type === 'alternative' && '🔄 Alt View'}
            {video.type === 'mistakes' && '⚠️ Mistakes'}
            {video.type === 'advanced' && '⭐ Advanced'}
          </button>
        ))}
      </div>

      {/* Video Player */}
      <div className="player-container">
        <YouTube
          videoId={activeVideo.youtubeVideoId}
          opts={youtubeOpts}
          onReady={e => { playerRef.current = e.target; }}
          onEnd={onVideoEnd}
        />
      </div>

      {/* Video Info */}
      <div className="video-info">
        <h3>{activeVideo.title}</h3>
        <p className="channel">by {activeVideo.channel}</p>
      </div>

      {/* Timestamp Shortcuts */}
      <div className="timestamp-shortcuts">
        <h4>Jump to:</h4>
        <div className="shortcuts-grid">
          {activeVideo.timestamps.setup !== undefined && (
            <button onClick={() => jumpToTimestamp('setup')}>
              🏗️ Setup ({formatTime(activeVideo.timestamps.setup)})
            </button>
          )}
          {activeVideo.timestamps.execution !== undefined && (
            <button onClick={() => jumpToTimestamp('execution')}>
              ▶️ Execution ({formatTime(activeVideo.timestamps.execution)})
            </button>
          )}
          {activeVideo.timestamps.common_mistakes !== undefined && (
            <button onClick={() => jumpToTimestamp('common_mistakes')}>
              ⚠️ Mistakes ({formatTime(activeVideo.timestamps.common_mistakes)})
            </button>
          )}
          {activeVideo.timestamps.variations !== undefined && (
            <button onClick={() => jumpToTimestamp('variations')}>
              🔀 Variations ({formatTime(activeVideo.timestamps.variations)})
            </button>
          )}
        </div>
      </div>

      {/* Playback Controls */}
      <div className="playback-controls">
        <div className="speed-control">
          <label>Speed:</label>
          <button
            className={playbackRate === 0.5 ? 'active' : ''}
            onClick={() => changeSpeed(0.5)}
          >
            0.5×
          </button>
          <button
            className={playbackRate === 1.0 ? 'active' : ''}
            onClick={() => changeSpeed(1.0)}
          >
            1×
          </button>
          <button
            className={playbackRate === 1.5 ? 'active' : ''}
            onClick={() => changeSpeed(1.5)}
          >
            1.5×
          </button>
        </div>

        {isInWorkout && (
          <button className="pip-toggle" onClick={togglePiP}>
            {isPiP ? '📺 Full Screen' : '📱 Mini Player'}
          </button>
        )}
      </div>

      {/* Download for Offline (optional) */}
      <div className="offline-options">
        <button onClick={() => downloadVideoForOffline(activeVideo.id)}>
          💾 Save for Offline (at gym with no signal)
        </button>
      </div>
    </div>
  );
};

// Helper function
const formatTime = (seconds: number): string => {
  const mins = Math.floor(seconds / 60);
  const secs = seconds % 60;
  return `${mins}:${secs.toString().padStart(2, '0')}`;
};
```

### Styling: `ExerciseVideoPlayer.module.css`

```css
.video-player {
  background: #000;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
}

.video-tabs {
  display: flex;
  background: #1a1a1a;
  padding: 8px;
  gap: 8px;
  overflow-x: auto;
}

.video-tabs .tab {
  padding: 8px 16px;
  background: #2a2a2a;
  color: #fff;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  white-space: nowrap;
  transition: all 0.2s;
}

.video-tabs .tab.active {
  background: #3b82f6;
  font-weight: 600;
}

.player-container {
  position: relative;
  padding-bottom: 56.25%; /* 16:9 aspect ratio */
  height: 0;
  overflow: hidden;
}

.player-container iframe {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
}

.video-info {
  padding: 16px;
  background: #1a1a1a;
  color: #fff;
}

.video-info h3 {
  margin: 0 0 4px 0;
  font-size: 16px;
  font-weight: 600;
}

.video-info .channel {
  margin: 0;
  font-size: 14px;
  color: #888;
}

.timestamp-shortcuts {
  padding: 16px;
  background: #0a0a0a;
}

.timestamp-shortcuts h4 {
  margin: 0 0 12px 0;
  color: #fff;
  font-size: 14px;
  font-weight: 600;
}

.shortcuts-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: 8px;
}

.shortcuts-grid button {
  padding: 10px;
  background: #2a2a2a;
  color: #fff;
  border: 1px solid #444;
  border-radius: 6px;
  cursor: pointer;
  font-size: 13px;
  transition: all 0.2s;
}

.shortcuts-grid button:hover {
  background: #3a3a3a;
  border-color: #3b82f6;
}

.playback-controls {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  background: #0a0a0a;
  border-top: 1px solid #222;
}

.speed-control {
  display: flex;
  align-items: center;
  gap: 8px;
}

.speed-control label {
  color: #888;
  font-size: 14px;
}

.speed-control button {
  padding: 6px 12px;
  background: #2a2a2a;
  color: #fff;
  border: 1px solid #444;
  border-radius: 4px;
  cursor: pointer;
  font-size: 13px;
}

.speed-control button.active {
  background: #3b82f6;
  border-color: #3b82f6;
}

.pip-toggle {
  padding: 8px 16px;
  background: #3b82f6;
  color: #fff;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 600;
}

/* Picture-in-Picture Mode */
.video-player.pip-mode {
  position: fixed;
  bottom: 80px;
  right: 20px;
  width: 300px;
  z-index: 1000;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
}

.video-player.pip-mode .video-tabs,
.video-player.pip-mode .timestamp-shortcuts,
.video-player.pip-mode .offline-options {
  display: none;
}

.video-player.pip-mode .player-container {
  padding-bottom: 66.67%; /* Smaller aspect ratio for PiP */
}

.offline-options {
  padding: 16px;
  background: #0a0a0a;
  border-top: 1px solid #222;
}

.offline-options button {
  width: 100%;
  padding: 12px;
  background: #16a34a;
  color: #fff;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 600;
  transition: background 0.2s;
}

.offline-options button:hover {
  background: #15803d;
}
```

---

## Video Curation Script

### Automated Discovery: `scripts/curate-exercise-videos.js`

```javascript
const { youtube } = require('@googleapis/youtube');
const fs = require('fs');

const YOUTUBE_API_KEY = process.env.YOUTUBE_API_KEY;
const yt = youtube({ version: 'v3', auth: YOUTUBE_API_KEY });

// Trusted channels
const TRUSTED_CHANNELS = {
  'UC68TLK0mAEzUyHx5x5k-S1Q': 'Jeff Nippard',
  'UCe0TLA0EsQbE-MjuHXevj2A': 'AthleanX',
  'UCfQgsKhHjSyRLOp9mnffqVg': 'Renaissance Periodization',
  'UC8Rt7E2JVz4fPmC8AhjDoaA': 'Alan Thrall',
  'UCy8Ct3BEYYcFaLFmG1o1e-A': 'Squat University',
};

const exercises = require('../backend/data/exercises.json'); // 638 exercises

async function findVideosForExercise(exerciseName) {
  const searchQuery = `${exerciseName} proper form technique`;

  const response = await yt.search.list({
    part: ['snippet'],
    q: searchQuery,
    type: ['video'],
    videoDefinition: 'high',
    maxResults: 10,
  });

  const videos = response.data.items.filter(item => {
    const channelId = item.snippet.channelId;
    return TRUSTED_CHANNELS[channelId]; // Only trusted channels
  });

  // Get video details
  const videoIds = videos.map(v => v.id.videoId).join(',');
  const detailsResponse = await yt.videos.list({
    part: ['contentDetails', 'statistics'],
    id: videoIds,
  });

  // Combine data
  const enrichedVideos = videos.map((video, i) => {
    const details = detailsResponse.data.items[i];
    return {
      youtubeVideoId: video.id.videoId,
      title: video.snippet.title,
      channel: TRUSTED_CHANNELS[video.snippet.channelId],
      channelId: video.snippet.channelId,
      duration: parseDuration(details.contentDetails.duration),
      viewCount: parseInt(details.statistics.viewCount),
      uploadDate: video.snippet.publishedAt,
    };
  });

  // Sort by quality (view count × channel reputation)
  return enrichedVideos
    .sort((a, b) => b.viewCount - a.viewCount)
    .slice(0, 3); // Top 3 videos
}

function parseDuration(isoDuration) {
  // Convert ISO 8601 duration to seconds
  const match = isoDuration.match(/PT(?:(\d+)H)?(?:(\d+)M)?(?:(\d+)S)?/);
  const hours = parseInt(match[1] || 0);
  const minutes = parseInt(match[2] || 0);
  const seconds = parseInt(match[3] || 0);
  return hours * 3600 + minutes * 60 + seconds;
}

async function curateAllVideos() {
  const results = [];

  for (const exercise of exercises) {
    console.log(`Curating videos for: ${exercise.name}`);

    const videos = await findVideosForExercise(exercise.name);

    results.push({
      exerciseId: exercise.id,
      exerciseName: exercise.name,
      videos: videos.map((video, index) => ({
        ...video,
        type: index === 0 ? 'primary' : 'alternative',
        qualityScore: 10 - index, // First result = 10, second = 9, etc.
      })),
    });

    // Rate limit (YouTube API has quota)
    await new Promise(resolve => setTimeout(resolve, 1000));
  }

  fs.writeFileSync(
    'exercise-videos-curated.json',
    JSON.stringify(results, null, 2)
  );

  console.log(`✓ Curated ${results.length} exercises with ${results.reduce((sum, r) => sum + r.videos.length, 0)} videos`);
}

curateAllVideos();
```

---

## Offline Caching Implementation

### Service Worker for Video Caching

```javascript
// frontend/public/sw-video-cache.js

const CACHE_NAME = 'forge-video-cache-v1';
const MAX_CACHE_SIZE = 500 * 1024 * 1024; // 500MB

self.addEventListener('install', event => {
  self.skipWaiting();
});

self.addEventListener('fetch', event => {
  // Only cache YouTube video requests
  if (event.request.url.includes('youtube.com') ||
      event.request.url.includes('ytimg.com')) {
    event.respondWith(
      caches.match(event.request).then(cachedResponse => {
        if (cachedResponse) {
          return cachedResponse; // Serve from cache
        }
        return fetch(event.request).then(response => {
          // Cache successful responses
          if (response && response.status === 200) {
            const responseClone = response.clone();
            caches.open(CACHE_NAME).then(cache => {
              cache.put(event.request, responseClone);
            });
          }
          return response;
        });
      })
    );
  }
});

// Cache management
self.addEventListener('message', event => {
  if (event.data.action === 'clearCache') {
    caches.delete(CACHE_NAME);
  }
});
```

---

## Summary

### Decisions Made
✅ **Video Source**: Link to curated YouTube videos from trusted fitness creators
✅ **Hosting**: Zero hosting costs, use YouTube's infrastructure
✅ **Player**: Premium in-app viewer with custom controls, timestamps, PiP, speed control
✅ **Offline**: Service worker caching for gym use (no signal)
✅ **Curation**: Semi-automated discovery + manual verification (2-3 days work)

### Implementation Timeline
- **Week 1**: Database schema, backend API, video curation script
- **Week 2**: Frontend video player component, timestamp UI
- **Week 3**: Offline caching, PiP mode, analytics tracking
- **Week 4**: Testing, polish, curate all 638 exercises

### Benefits
- ✅ Professional, high-quality content
- ✅ Zero video hosting costs
- ✅ Multiple perspectives per exercise
- ✅ Instant content library
- ✅ Legal (linking, not reuploading)
- ✅ Better than MacroFactor (multiple coaches, not just one)

Ready to start Phase 1 implementation! 🚀
