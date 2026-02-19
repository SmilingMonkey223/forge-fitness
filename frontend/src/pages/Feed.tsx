import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '../services/api'
import { FeedPost } from '../components/FeedPost'

interface FeedPageProps {
  onLogout: () => void
}

export function Feed({ onLogout }: FeedPageProps) {
  const navigate = useNavigate()
  const [feedType, setFeedType] = useState<'personalized' | 'latest'>('personalized')
  const [posts, setPosts] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [hasMore, setHasMore] = useState(true)
  const [offset, setOffset] = useState(0)

  useEffect(() => {
    loadFeed(true)
  }, [feedType])

  const loadFeed = async (reset = false) => {
    try {
      setLoading(true)
      const currentOffset = reset ? 0 : offset
      const endpoint = feedType === 'personalized'
        ? `/api/social/feed/personalized?limit=20&offset=${currentOffset}`
        : `/api/social/feed/latest?limit=20&offset=${currentOffset}`

      const response = await api.get(endpoint)

      if (reset) {
        setPosts(response.posts)
        setOffset(20)
      } else {
        setPosts([...posts, ...response.posts])
        setOffset(currentOffset + 20)
      }

      setHasMore(response.posts.length === 20)
    } catch (error) {
      console.error('Failed to load feed:', error)
      // Use mock data for demonstration
      loadMockData(reset)
    } finally {
      setLoading(false)
    }
  }

  const loadMockData = (reset: boolean) => {
    const mockPosts = [
      {
        id: '1',
        user_id: 'user1',
        username: 'JohnDoe',
        post_type: 'workout',
        title: 'Chest Day Completed! 💪',
        description: 'Crushed a great chest workout today:\n- Bench Press: 225 lbs x 5 reps\n- Incline DB Press: 80 lbs x 10 reps\n- Cable Flyes: 4 sets\n\nFeeling strong!',
        visibility: 'everyone',
        created_at: new Date(Date.now() - 2 * 60 * 60 * 1000).toISOString(),
        kudos_count: 12,
        comment_count: 3,
        has_kudos_from_user: false,
      },
      {
        id: '2',
        user_id: 'user2',
        username: 'SarahFit',
        post_type: 'pr',
        title: 'New Squat PR! 🎉',
        description: 'Just hit a new personal record on squats: 315 lbs x 3 reps!\n\nBeen working towards this for months. So happy right now!',
        visibility: 'everyone',
        created_at: new Date(Date.now() - 5 * 60 * 60 * 1000).toISOString(),
        kudos_count: 28,
        comment_count: 7,
        has_kudos_from_user: false,
      },
      {
        id: '3',
        user_id: 'user3',
        username: 'MikeLift',
        post_type: 'achievement',
        title: 'Century Club Achievement Unlocked! 💯',
        description: 'Just completed my 100th workout on FORGE! Consistency is key 🔥',
        visibility: 'everyone',
        created_at: new Date(Date.now() - 1 * 24 * 60 * 60 * 1000).toISOString(),
        kudos_count: 45,
        comment_count: 12,
        has_kudos_from_user: true,
      },
      {
        id: '4',
        user_id: 'user4',
        username: 'EmilyStrong',
        post_type: 'workout',
        title: 'Leg Day = Best Day',
        description: 'Destroyed legs today with high volume training. Can barely walk to my car 😅\n\nTotal volume: 15,000 lbs',
        visibility: 'everyone',
        created_at: new Date(Date.now() - 2 * 24 * 60 * 60 * 1000).toISOString(),
        kudos_count: 18,
        comment_count: 5,
        has_kudos_from_user: false,
      },
    ]

    if (reset) {
      setPosts(mockPosts)
    } else {
      setPosts([...posts, ...mockPosts])
    }
    setHasMore(false)
  }

  const handleLogout = () => {
    api.logout()
    onLogout()
  }

  return (
    <div className="min-h-screen bg-background">
      {/* Header */}
      <div className="bg-surface border-b border-border sticky top-0 z-10">
        <div className="max-w-4xl mx-auto px-4 py-4">
          <div className="flex justify-between items-center mb-4">
            <h1 className="text-2xl font-bold text-text-primary">FORGE</h1>
            <div className="flex items-center gap-4">
              <button
                onClick={() => navigate('/')}
                className="text-text-secondary hover:text-primary transition-colors"
              >
                Dashboard
              </button>
              <button
                onClick={() => navigate('/workout')}
                className="px-4 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
              >
                Start Workout
              </button>
              <button
                onClick={handleLogout}
                className="text-text-secondary hover:text-danger transition-colors"
              >
                Log Out
              </button>
            </div>
          </div>

          {/* Feed Type Toggle */}
          <div className="flex gap-2">
            <button
              onClick={() => setFeedType('personalized')}
              className={`px-4 py-2 rounded-lg transition-colors ${
                feedType === 'personalized'
                  ? 'bg-primary text-white'
                  : 'bg-background text-text-secondary hover:bg-surface-light'
              }`}
            >
              Following
            </button>
            <button
              onClick={() => setFeedType('latest')}
              className={`px-4 py-2 rounded-lg transition-colors ${
                feedType === 'latest'
                  ? 'bg-primary text-white'
                  : 'bg-background text-text-secondary hover:bg-surface-light'
              }`}
            >
              Latest
            </button>
          </div>
        </div>
      </div>

      {/* Feed Content */}
      <div className="max-w-4xl mx-auto px-4 py-6">
        {loading && posts.length === 0 ? (
          <div className="text-center py-12">
            <div className="text-2xl text-text-secondary">Loading feed...</div>
          </div>
        ) : (
          <>
            {posts.length === 0 ? (
              <div className="bg-surface border border-border rounded-lg p-12 text-center">
                <div className="text-4xl mb-4">👋</div>
                <h3 className="text-xl font-bold text-text-primary mb-2">
                  Welcome to FORGE Social!
                </h3>
                <p className="text-text-secondary mb-6">
                  {feedType === 'personalized'
                    ? "Follow other users to see their workouts and achievements here"
                    : "No posts yet. Be the first to share your workout!"}
                </p>
                <button
                  onClick={() => navigate('/workout')}
                  className="px-6 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
                >
                  Start Your First Workout
                </button>
              </div>
            ) : (
              <>
                {posts.map((post) => (
                  <FeedPost
                    key={post.id}
                    post={post}
                    onKudos={() => loadFeed(true)}
                    onComment={() => loadFeed(true)}
                  />
                ))}

                {hasMore && (
                  <div className="text-center py-6">
                    <button
                      onClick={() => loadFeed(false)}
                      disabled={loading}
                      className="px-6 py-2 bg-surface border border-border text-text-primary rounded-lg hover:border-primary transition-colors disabled:opacity-50"
                    >
                      {loading ? 'Loading...' : 'Load More'}
                    </button>
                  </div>
                )}
              </>
            )}
          </>
        )}
      </div>
    </div>
  )
}
