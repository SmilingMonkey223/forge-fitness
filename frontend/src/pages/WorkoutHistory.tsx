import { useState, useCallback } from 'react'
import { useQuery } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { WorkoutSummary, Workout } from '@/types'

type Filter = 'all' | 'week' | 'month'

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
  const [filter, setFilter] = useState<Filter>('all')
  const [expandedId, setExpandedId] = useState<string | null>(null)
  const [expandedWorkout, setExpandedWorkout] = useState<Workout | null>(null)
  const [loadingDetail, setLoadingDetail] = useState(false)
  const PAGE_SIZE = 20

  const { data: workouts, isLoading, error } = useQuery<WorkoutSummary[]>({
    queryKey: ['workouts', filter],
    queryFn: () => api.getWorkouts({ limit: PAGE_SIZE }),
  })

  const filteredWorkouts = (workouts || []).filter((w) => {
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
            <button
              onClick={() => toggleExpand(w.id)}
              className="tap-target w-full text-left px-4 py-4 hover:bg-surface-elevated/30 transition-colors"
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

            {/* Expanded detail */}
            {expandedId === w.id && (
              <div className="border-t border-surface-elevated px-4 py-3">
                {loadingDetail ? (
                  <div className="text-center py-4 text-text-muted text-sm">Loading details...</div>
                ) : expandedWorkout ? (
                  <div className="space-y-3">
                    {Object.entries(exerciseSets).map(([, sets]) => {
                      // We don't have exercise names in the set data, show exercise_id
                      return (
                        <div key={sets[0].exercise_id}>
                          <div className="text-sm font-medium text-primary mb-1">
                            Exercise
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
      </div>
    </div>
  )
}
