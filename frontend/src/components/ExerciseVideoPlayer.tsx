import React, { useState, useEffect, useRef } from 'react';
import YouTube, { YouTubeProps } from 'react-youtube';

interface ExerciseVideo {
  id: string;
  type: 'primary' | 'alternative' | 'mistakes' | 'advanced';
  youtube_video_id: string;
  embed_url: string;
  title: string;
  channel: string;
  duration?: number;
  timestamps: {
    setup?: number;
    execution?: number;
    common_mistakes?: number;
    variations?: number;
  };
  quality_score: number;
}

interface Props {
  exerciseId: string;
  exerciseName: string;
  isInWorkout?: boolean; // Show PiP controls if true
  onClose?: () => void;
}

export const ExerciseVideoPlayer: React.FC<Props> = ({
  exerciseId,
  exerciseName,
  isInWorkout = false,
  onClose,
}) => {
  const [videos, setVideos] = useState<ExerciseVideo[]>([]);
  const [activeVideo, setActiveVideo] = useState<ExerciseVideo | null>(null);
  const [playbackRate, setPlaybackRate] = useState(1.0);
  const [isPiP, setIsPiP] = useState(false);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const playerRef = useRef<any>(null);

  // Fetch videos on mount
  useEffect(() => {
    fetchVideos();
  }, [exerciseId]);

  const fetchVideos = async () => {
    try {
      setLoading(true);
      const response = await fetch(`/api/exercises/${exerciseId}/videos`);

      if (!response.ok) {
        throw new Error('Failed to fetch videos');
      }

      const data = await response.json();
      setVideos(data.videos);

      // Set primary video as default, or first available
      const primary = data.videos.find((v: ExerciseVideo) => v.type === 'primary');
      setActiveVideo(primary || data.videos[0] || null);
      setLoading(false);
    } catch (err) {
      console.error('Error fetching videos:', err);
      setError('Failed to load exercise videos');
      setLoading(false);
    }
  };

  // YouTube player options
  const youtubeOpts: YouTubeProps['opts'] = {
    height: isPiP ? '200' : '390',
    width: isPiP ? '300' : '100%',
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
      const accessToken = localStorage.getItem('access_token');
      fetch(`/api/videos/${activeVideo.id}/view`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${accessToken}`,
        },
        body: JSON.stringify({
          watch_time_seconds: activeVideo.duration || 0,
          completed: true,
        }),
      }).catch(err => console.error('Failed to track view:', err));
    }
  };

  const formatTime = (seconds: number): string => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  };

  const getVideoTypeLabel = (type: string) => {
    switch (type) {
      case 'primary':
        return '📹 Technique';
      case 'alternative':
        return '🔄 Alt View';
      case 'mistakes':
        return '⚠️ Mistakes';
      case 'advanced':
        return '⭐ Advanced';
      default:
        return type;
    }
  };

  if (loading) {
    return (
      <div className="bg-gray-900 rounded-xl p-8 text-center">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-blue-500 mx-auto"></div>
        <p className="text-gray-400 mt-4">Loading exercise videos...</p>
      </div>
    );
  }

  if (error || !activeVideo) {
    return (
      <div className="bg-gray-900 rounded-xl p-8 text-center">
        <p className="text-red-500">{error || 'No videos available for this exercise'}</p>
        {onClose && (
          <button
            onClick={onClose}
            className="mt-4 px-4 py-2 bg-gray-700 text-white rounded-lg hover:bg-gray-600"
          >
            Close
          </button>
        )}
      </div>
    );
  }

  return (
    <div className={`video-player bg-black rounded-xl overflow-hidden shadow-2xl ${isPiP ? 'fixed bottom-20 right-5 w-80 z-50' : ''}`}>
      {/* Close button (top right) */}
      {onClose && (
        <button
          onClick={onClose}
          className="absolute top-2 right-2 z-10 bg-black/50 text-white rounded-full w-8 h-8 flex items-center justify-center hover:bg-black/70"
        >
          ✕
        </button>
      )}

      {/* Video Tabs */}
      {!isPiP && videos.length > 1 && (
        <div className="flex gap-2 p-2 bg-gray-900 overflow-x-auto">
          {videos.map(video => (
            <button
              key={video.id}
              className={`px-4 py-2 rounded-lg text-sm font-medium whitespace-nowrap transition-all ${
                activeVideo.id === video.id
                  ? 'bg-blue-600 text-white'
                  : 'bg-gray-800 text-gray-300 hover:bg-gray-700'
              }`}
              onClick={() => setActiveVideo(video)}
            >
              {getVideoTypeLabel(video.type)}
            </button>
          ))}
        </div>
      )}

      {/* Video Player */}
      <div className="relative" style={{ paddingBottom: isPiP ? '0' : '56.25%', height: isPiP ? '200px' : '0' }}>
        <YouTube
          videoId={activeVideo.youtube_video_id}
          opts={youtubeOpts}
          onReady={e => { playerRef.current = e.target; }}
          onEnd={onVideoEnd}
          className={isPiP ? '' : 'absolute top-0 left-0 w-full h-full'}
        />
      </div>

      {/* Video Info */}
      {!isPiP && (
        <>
          <div className="p-4 bg-gray-900">
            <h3 className="text-white font-semibold text-lg mb-1">{activeVideo.title}</h3>
            <p className="text-gray-400 text-sm">by {activeVideo.channel}</p>
          </div>

          {/* Timestamp Shortcuts */}
          {Object.keys(activeVideo.timestamps).length > 0 && (
            <div className="p-4 bg-black">
              <h4 className="text-white text-sm font-semibold mb-3">Jump to:</h4>
              <div className="grid grid-cols-2 gap-2">
                {activeVideo.timestamps.setup !== undefined && (
                  <button
                    onClick={() => jumpToTimestamp('setup')}
                    className="px-3 py-2 bg-gray-800 text-white rounded-lg text-sm hover:bg-gray-700 border border-gray-700"
                  >
                    🏗️ Setup ({formatTime(activeVideo.timestamps.setup)})
                  </button>
                )}
                {activeVideo.timestamps.execution !== undefined && (
                  <button
                    onClick={() => jumpToTimestamp('execution')}
                    className="px-3 py-2 bg-gray-800 text-white rounded-lg text-sm hover:bg-gray-700 border border-gray-700"
                  >
                    ▶️ Execution ({formatTime(activeVideo.timestamps.execution)})
                  </button>
                )}
                {activeVideo.timestamps.common_mistakes !== undefined && (
                  <button
                    onClick={() => jumpToTimestamp('common_mistakes')}
                    className="px-3 py-2 bg-gray-800 text-white rounded-lg text-sm hover:bg-gray-700 border border-gray-700"
                  >
                    ⚠️ Mistakes ({formatTime(activeVideo.timestamps.common_mistakes)})
                  </button>
                )}
                {activeVideo.timestamps.variations !== undefined && (
                  <button
                    onClick={() => jumpToTimestamp('variations')}
                    className="px-3 py-2 bg-gray-800 text-white rounded-lg text-sm hover:bg-gray-700 border border-gray-700"
                  >
                    🔀 Variations ({formatTime(activeVideo.timestamps.variations)})
                  </button>
                )}
              </div>
            </div>
          )}

          {/* Playback Controls */}
          <div className="flex justify-between items-center p-4 bg-black border-t border-gray-800">
            <div className="flex items-center gap-2">
              <label className="text-gray-400 text-sm">Speed:</label>
              <button
                className={`px-3 py-1 rounded text-sm ${playbackRate === 0.5 ? 'bg-blue-600 text-white' : 'bg-gray-800 text-gray-300 hover:bg-gray-700'}`}
                onClick={() => changeSpeed(0.5)}
              >
                0.5×
              </button>
              <button
                className={`px-3 py-1 rounded text-sm ${playbackRate === 1.0 ? 'bg-blue-600 text-white' : 'bg-gray-800 text-gray-300 hover:bg-gray-700'}`}
                onClick={() => changeSpeed(1.0)}
              >
                1×
              </button>
              <button
                className={`px-3 py-1 rounded text-sm ${playbackRate === 1.5 ? 'bg-blue-600 text-white' : 'bg-gray-800 text-gray-300 hover:bg-gray-700'}`}
                onClick={() => changeSpeed(1.5)}
              >
                1.5×
              </button>
            </div>

            {isInWorkout && (
              <button
                className="px-4 py-2 bg-blue-600 text-white rounded-lg text-sm font-semibold hover:bg-blue-700"
                onClick={togglePiP}
              >
                {isPiP ? '📺 Full' : '📱 Mini'}
              </button>
            )}
          </div>
        </>
      )}

      {/* PiP mode: Show only minimize button */}
      {isPiP && (
        <div className="p-2 bg-gray-900 flex justify-between items-center">
          <span className="text-white text-xs font-medium truncate">{exerciseName}</span>
          <button
            onClick={togglePiP}
            className="px-3 py-1 bg-blue-600 text-white rounded text-xs font-semibold hover:bg-blue-700"
          >
            Expand
          </button>
        </div>
      )}
    </div>
  );
};
