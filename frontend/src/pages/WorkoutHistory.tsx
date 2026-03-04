import { useState, useCallback } from 'react'
import { useQuery, useQueryClient } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { WorkoutSummary, Workout } from '@/types'

type Filter = 'all' | 'week' | 'month'
const PAGE_SIZE = 20
const FILTERED_LIMIT = 100

function formatDuration(seconds?: number): string {
  if (!seconds) return '-'
  const m = Math.floor(seconds / 60)
  return m < 60 ? `${m}min` : `${Math.floor(m / 60)}h ${m % 60}m`
}

function formatDate(dateStr: string): string {
  const d = new Date(dateStr)
  return d.toLocaleDateString('en-US', { weekday: 'short', month: 'short', day: 'numeric' })
}

function formatVolume(kg: number): string {
  if (kg >= 1000) return `${(kg / 1000).toFixed(1)}t`
  return `${Math.round(kg)}kg`
}

export default function WorkoutHistory() {
  const queryClient = useQueryClient()
  const [filter, setFilter] = useState<Filter>('all')
  const [expandedId, setExpandedId] = useState<string | null>(null)
  const [expandedWorkout, setExpandedWorkout] = useState<Workout | null>(null)
  const [loadingDetail, setLoadingDetail] = useState(false)
  const [extraWorkouts, setExtraWorkouts] = useState<WorkoutSummary[]>([])
  const [offset, setOffset] = useState(0)
  const [loadingMore, setLoadingMore] = useState(false)
  const [hasMore, setHasMore] = useState(true)
  const [deletingId, setDeletingId] = useState<string | null>(null)
  const [confirmDeleteId, setConfirmDeleteId] = useState<string | null>(null)

  const fetchLimit = filter === 'all' ? PAGE_SIZE : FILTERED_LIMIT

  const { data: workouts, isLoading, error } = useQuery<WorkoutSummary[]>({
    queryKey: ['workouts', filter],
    queryFn: () => api.getWorkouts({ limit: fetchLimit }),
  })

  const allWorkouts = [
    ...(workouts || []),
    ...extraWorkouts.filter((ew) => !(workouts || []).some((w) => w.id === ew.id)),
  ]

  const filteredWorkouts = allWorkouts.filter((w) => {
    if (filter === 'all') return true
    const d = new Date(w.started_at)
    const now = new Date()
    if (filter === 'week') {
      const weekAgo = new Date(now)
      weekAgo.setDate(weekAgo.getDate() - 7)
      return d >= weekAgo
    }
    if (filter === 'month') {
      const monthAgo = new Date(now)
      monthAgo.setMonth(monthAgo.getMonth() - 1)
      return d >= monthAgo
    }
    return true
  })

  const loadMore = useCallback(async () => {
    setLoadingMore(true)
    try {
      const nextOffset = offset + PAGE_SIZE
      const more = await api.getWorkouts({ limit: PAGE_SIZE, offset: nextOffset })
      if (more.length < PAGE_SIZE) {
        setHasMore(false)
      }
      setExtraWorkouts((prev) => [...prev, ...more])
      setOffset(nextOffset)
    } catch {
      // silently fail, user can retry
    } finally {
      setLoadingMore(false)
    }
  }, [offset])

  const handleDelete = useCallback(async (id: string) => {
    setDeletingId(id)
    try {
      await api.deleteWorkout(id)
      // Optimistically remove from extra workouts
      setExtraWorkouts((prev) => prev.filter((w) => w.id !== id))
      // Invalidate the main query to remove from cached data
      queryClient.setQueryData<WorkoutSummary[]>(['workouts', filter], (old) =>
        old ? old.filter((w) => w.id !== id) : []
      )
      if (expandedId === id) {
        setExpandedId(null)
        setExpandedWorkout(null)
      }
      setConfirmDeleteId(null)
    } catch {
      // keep the item visible on failure
    } finally {
      setDeletingId(null)
    }
  }, [filter, expandedId, queryClient])

  const toggleExpand = useCallback(async (id: string) => {
    if (expandedId === id) {
      setExpandedId(null)
      setExpandedWorkout(null)
      return
    }
    setExpandedId(id)
    setLoadingDetail(true)
    try {
      const w = await api.getWorkout(id)
      setExpandedWorkout(w)
    } catch {
      setExpandedWorkout(null)
    } finally {
      setLoadingDetail(false)
    }
  }, [expandedId])

  // Group sets by exercise for expanded view
  const exerciseSets = expandedWorkout
    ? expandedWorkout.sets.reduce<Record<string, typeof expandedWorkout.sets>>((acc, set) => {
        const key = set.exercise_id
        if (!acc[key]) acc[key] = []
        acc[key].push(set)
        return acc
      }, {})
    : {}

  const filters: { key: Filter; label: string }[] = [
    { key: 'all', label: 'All' },
    { key: 'week', label: 'This Week' },
    { key: 'month', label: 'This Month' },
  ]

  return (
    <div className="min-h-screen bg-background pb-20">
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-2xl mx-auto px-4 py-4">
          <h1 className="text-2xl font-bold">Workout History</h1>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-4 space-y-4">
        {/* Filter chips */}
        <div className="flex gap-2">
          {filters.map((f) => (
            <button
              key={f.key}
              onClick={() => setFilter(f.key)}
              className={`tap-target px-4 py-2 rounded-button text-sm font-medium transition-all ${
                filter === f.key
                  ? 'bg-primary text-white'
                  : 'bg-surface-elevated text-text-secondary hover:text-text-primary'
              }`}
            >
              {f.label}
            </button>
          ))}
        </div>

        {/* Loading */}
        {isLoading && (
          <div className="text-center py-12 text-text-muted">Loading workouts...</div>
        )}

        {/* Error */}
        {error && (
          <div className="card text-center py-8">
            <div className="text-danger mb-2">Failed to load workouts</div>
            <button onClick={() => window.location.reload()} className="text-primary text-sm">
              Retry
            </button>
          </div>
        )}

        {/* Empty */}
        {!isLoading && !error && filteredWorkouts.length === 0 && (
          <div className="card text-center py-12">
            <div className="text-text-muted mb-2">No workouts found</div>
            <div className="text-text-muted text-sm">
              Start your first workout from the dashboard!
            </div>
          </div>
        )}

        {/* Workout list */}
        {filteredWorkouts.map((w) => (
          <div key={w.id} className="card p-0 overflow-hidden">
            <div className="flex items-stretch">
              <button
                onClick={() => toggleExpand(w.id)}
                className="tap-target flex-1 text-left px-4 py-4 hover:bg-surface-elevated/30 transition-colors"
              >
                <div className="flex items-start justify-between mb-2">
                  <div>
                    <div className="font-bold text-text-primary">{w.name || 'Workout'}</div>
                    <div className="text-xs text-text-muted">{formatDate(w.started_at)}</div>
                  </div>
                  <div className={`text-xs px-2 py-0.5 rounded-full ${
                    w.status === 'completed' ? 'bg-success/20 text-success' : 'bg-warning/20 text-warning'
                  }`}>
                    {w.status}
                  </div>
                </div>
                <div className="grid grid-cols-4 gap-2 text-center">
                  <div>
                    <div className="text-xs text-text-muted">Duration</div>
                    <div className="text-sm font-mono font-bold">{formatDuration(w.duration_seconds)}</div>
                  </div>
                  <div>
                    <div className="text-xs text-text-muted">Volume</div>
                    <div className="text-sm font-mono font-bold">{formatVolume(w.total_volume_kg)}</div>
                  </div>
                  <div>
                    <div className="text-xs text-text-muted">Exercises</div>
                    <div className="text-sm font-mono font-bold">{w.exercises_count}</div>
                  </div>
                  <div>
                    <div className="text-xs text-text-muted">Sets</div>
                    <div className="text-sm font-mono font-bold">{w.sets_count}</div>
                  </div>
                </div>
              </button>
              <button
                onClick={(e) => {
                  e.stopPropagation()
                  setConfirmDeleteId(confirmDeleteId === w.id ? null : w.id)
                }}
                className="flex items-center px-3 text-danger hover:bg-danger/10 transition-colors"
                aria-label="Delete workout"
              >
                <svg xmlns="http://www.w3.org/2000/svg" className="h-5 w-5" viewBox="0 0 20 20" fill="currentColor">
                  <path fillRule="evenodd" d="M9 2a1 1 0 00-.894.553L7.382 4H4a1 1 0 000 2v10a2 2 0 002 2h8a2 2 0 002-2V6a1 1 0 100-2h-3.382l-.724-1.447A1 1 0 0011 2H9zM7 8a1 1 0 012 0v6a1 1 0 11-2 0V8zm5-1a1 1 0 00-1 1v6a1 1 0 102 0V8a1 1 0 00-1-1z" clipRule="evenodd" />
                </svg>
              </button>
            </div>

            {/* Delete confirmation */}
            {confirmDeleteId === w.id && (
              <div className="border-t border-surface-elevated px-4 py-3 flex items-center justify-between bg-danger/5">
                <span className="text-sm text-danger font-medium">Delete this workout?</span>
                <div className="flex gap-2">
                  <button
                    onClick={() => setConfirmDeleteId(null)}
                    className="px-3 py-1 text-sm rounded-button bg-surface-elevated text-text-secondary hover:text-text-primary transition-colors"
                  >
                    Cancel
                  </button>
                  <button
                    onClick={() => handleDelete(w.id)}
                    disabled={deletingId === w.id}
                    className="px-3 py-1 text-sm rounded-button bg-danger text-white hover:bg-danger/80 transition-colors disabled:opacity-50"
                  >
                    {deletingId === w.id ? 'Deleting...' : 'Delete'}
                  </button>
                </div>
              </div>
            )}

            {/* Expanded detail */}
            {expandedId === w.id && (
              <div className="border-t border-surface-elevated px-4 py-3">
                {loadingDetail ? (
                  <div className="text-center py-4 text-text-muted text-sm">Loading details...</div>
                ) : expandedWorkout ? (
                  <div className="space-y-3">
                    {Object.entries(exerciseSets).map(([, sets]) => {
                      return (
                        <div key={sets[0].exercise_id}>
                          <div className="text-sm font-medium text-primary mb-1">
                            {sets[0].exercise_name || 'Unknown Exercise'}
                          </div>
                          <div className="space-y-0.5">
                            {sets.map((set) => (
                              <div key={set.id} className="flex items-center gap-2 text-sm text-text-secondary">
                                <span className={`w-5 text-xs text-center ${
                                  set.set_type === 'warmup' ? 'text-warning' : ''
                                }`}>
                                  {set.set_type === 'warmup' ? 'W' : set.set_order}
                                </span>
                                <span className="font-mono">
                                  {set.weight_kg}kg x {set.reps}
                                </span>
                                {set.rpe && <span className="text-text-muted text-xs">@{set.rpe}</span>}
                                {set.is_pr && (
                                  <span className="text-celebration text-xs">PR</span>
                                )}
                              </div>
                            ))}
                          </div>
                        </div>
                      )
                    })}
                  </div>
                ) : (
                  <div className="text-center py-4 text-text-muted text-sm">No details available</div>
                )}
              </div>
            )}
          </div>
        ))}

        {/* Load more button */}
        {!isLoading && !error && filteredWorkouts.length > 0 && filter === 'all' && hasMore && (
          <button
            onClick={loadMore}
            disabled={loadingMore}
            className="w-full py-3 text-center text-sm font-medium text-primary bg-surface-elevated rounded-button hover:bg-surface-elevated/80 transition-colors disabled:opacity-50"
          >
            {loadingMore ? 'Loading...' : 'Load more'}
          </button>
        )}
      </div>
    </div>
  )
}
