import { useState } from 'react'
import { api } from '../services/api'

interface FeedPostProps {
  post: {
    id: string
    user_id: string
    username: string
    avatar_url?: string
    post_type: string
    workout_id?: string
    title?: string
    description?: string
    image_url?: string
    visibility: string
    created_at: string
    kudos_count: number
    comment_count: number
    has_kudos_from_user: boolean
  }
  onKudos?: () => void
  onComment?: () => void
}

export function FeedPost({ post, onKudos, onComment }: FeedPostProps) {
  const [showComments, setShowComments] = useState(false)
  const [comments, setComments] = useState<any[]>([])
  const [newComment, setNewComment] = useState('')
  const [localKudosCount, setLocalKudosCount] = useState(post.kudos_count)
  const [hasGivenKudos, setHasGivenKudos] = useState(post.has_kudos_from_user)

  const formatRelativeTime = (dateString: string) => {
    const date = new Date(dateString)
    const now = new Date()
    const diffMs = now.getTime() - date.getTime()
    const diffMins = Math.floor(diffMs / 60000)
    const diffHours = Math.floor(diffMins / 60)
    const diffDays = Math.floor(diffHours / 24)

    if (diffMins < 1) return 'just now'
    if (diffMins < 60) return `${diffMins}m ago`
    if (diffHours < 24) return `${diffHours}h ago`
    if (diffDays < 7) return `${diffDays}d ago`
    if (diffDays < 30) return `${Math.floor(diffDays / 7)}w ago`
    return `${Math.floor(diffDays / 30)}mo ago`
  }

  const getPostIcon = () => {
    switch (post.post_type) {
      case 'workout': return '💪'
      case 'pr': return '🎉'
      case 'achievement': return '🏆'
      case 'progress_photo': return '📸'
      case 'milestone': return '⭐'
      default: return '📝'
    }
  }

  const handleGiveKudos = async () => {
    if (hasGivenKudos) return // Already given kudos (permanent)

    try {
      await api.post(`/api/social/posts/${post.id}/kudos`, {})
      setLocalKudosCount(localKudosCount + 1)
      setHasGivenKudos(true)
      onKudos?.()
    } catch (error) {
      console.error('Failed to give kudos:', error)
    }
  }

  const loadComments = async () => {
    try {
      const response = await api.get(`/api/social/posts/${post.id}/comments`)
      setComments(response)
      setShowComments(true)
    } catch (error) {
      console.error('Failed to load comments:', error)
    }
  }

  const handleAddComment = async () => {
    if (!newComment.trim()) return

    try {
      await api.post(`/api/social/posts/${post.id}/comments`, { content: newComment })
      setNewComment('')
      loadComments() // Reload comments
      onComment?.()
    } catch (error) {
      console.error('Failed to add comment:', error)
    }
  }

  return (
    <div className="bg-surface border border-border rounded-lg p-6 mb-4">
      {/* Header */}
      <div className="flex items-center mb-4">
        <div className="w-12 h-12 rounded-full bg-primary flex items-center justify-center text-white font-bold text-lg mr-3">
          {post.avatar_url ? (
            <img src={post.avatar_url} alt={post.username} className="w-full h-full rounded-full object-cover" />
          ) : (
            post.username[0].toUpperCase()
          )}
        </div>
        <div className="flex-1">
          <div className="font-bold text-text-primary">{post.username}</div>
          <div className="text-sm text-text-secondary flex items-center gap-2">
            <span>{getPostIcon()}</span>
            <span>{post.post_type.replace('_', ' ')}</span>
            <span>•</span>
            <span>{formatRelativeTime(post.created_at)}</span>
            {post.visibility !== 'everyone' && (
              <>
                <span>•</span>
                <span className="text-xs">🔒 {post.visibility}</span>
              </>
            )}
          </div>
        </div>
      </div>

      {/* Content */}
      {post.title && (
        <h3 className="text-lg font-bold text-text-primary mb-2">{post.title}</h3>
      )}
      {post.description && (
        <p className="text-text-secondary mb-4 whitespace-pre-wrap">{post.description}</p>
      )}

      {/* Image */}
      {post.image_url && (
        <img
          src={post.image_url}
          alt="Post content"
          className="w-full rounded-lg mb-4 max-h-96 object-cover"
        />
      )}

      {/* Actions */}
      <div className="flex items-center gap-6 border-t border-border pt-4">
        <button
          onClick={handleGiveKudos}
          disabled={hasGivenKudos}
          className={`flex items-center gap-2 transition-colors ${
            hasGivenKudos
              ? 'text-primary cursor-not-allowed'
              : 'text-text-secondary hover:text-primary'
          }`}
        >
          <span className="text-xl">{hasGivenKudos ? '👍' : '👍🏻'}</span>
          <span className="font-medium">{localKudosCount}</span>
          <span className="text-sm">Kudos</span>
        </button>

        <button
          onClick={() => showComments ? setShowComments(false) : loadComments()}
          className="flex items-center gap-2 text-text-secondary hover:text-primary transition-colors"
        >
          <span className="text-xl">💬</span>
          <span className="font-medium">{post.comment_count}</span>
          <span className="text-sm">Comments</span>
        </button>
      </div>

      {/* Comments Section */}
      {showComments && (
        <div className="mt-4 border-t border-border pt-4">
          {/* Comment Input */}
          <div className="flex gap-3 mb-4">
            <input
              type="text"
              value={newComment}
              onChange={(e) => setNewComment(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleAddComment()}
              placeholder="Write a comment..."
              className="flex-1 bg-background border border-border rounded-lg px-4 py-2 text-text-primary placeholder-text-secondary"
            />
            <button
              onClick={handleAddComment}
              disabled={!newComment.trim()}
              className="px-4 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
            >
              Post
            </button>
          </div>

          {/* Comments List */}
          <div className="space-y-3">
            {comments.map((comment) => (
              <div key={comment.id} className="flex gap-3">
                <div className="w-8 h-8 rounded-full bg-primary flex items-center justify-center text-white text-sm">
                  {comment.avatar_url ? (
                    <img src={comment.avatar_url} alt={comment.username} className="w-full h-full rounded-full object-cover" />
                  ) : (
                    comment.username[0].toUpperCase()
                  )}
                </div>
                <div className="flex-1 bg-background rounded-lg p-3">
                  <div className="flex items-center gap-2 mb-1">
                    <span className="font-medium text-text-primary text-sm">{comment.username}</span>
                    <span className="text-xs text-text-secondary">{formatRelativeTime(comment.created_at)}</span>
                  </div>
                  <p className="text-text-primary text-sm">{comment.content}</p>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  )
}
