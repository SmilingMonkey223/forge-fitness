import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useQuery } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { DashboardData } from '@/types'
import { useWorkoutStore } from '@/stores/workoutStore'

interface DashboardProps {
  onLogout: () => void
}

function MacroRing({
  label,
  consumed,
  target,
  color,
}: {
  label: string
  consumed: number
  target: number
  color: string
}) {
  const percentage = target > 0 ? Math.min((consumed / target) * 100, 100) : 0
  const circumference = 2 * Math.PI * 45
  const strokeDashoffset = circumference - (percentage / 100) * circumference

  return (
    <div className="flex flex-col items-center">
      <div className="relative w-24 h-24 sm:w-28 sm:h-28">
        <svg className="w-full h-full transform -rotate-90" viewBox="0 0 112 112">
          <circle cx="56" cy="56" r="45" stroke="#1E1E2E" strokeWidth="8" fill="none" />
          <circle
            cx="56" cy="56" r="45"
            stroke={color} strokeWidth="8" fill="none"
            strokeDasharray={circumference}
            strokeDashoffset={strokeDashoffset}
            strokeLinecap="round"
            className="transition-all duration-800 ease-out"
          />
        </svg>
        <div className="absolute inset-0 flex flex-col items-center justify-center">
          <div className="text-lg sm:text-xl font-bold font-mono">{Math.round(consumed)}</div>
          <div className="text-xs text-text-muted">/ {target}</div>
        </div>
      </div>
      <div className="text-sm text-text-secondary mt-2">{label}</div>
    </div>
  )
}

export default function Dashboard({ onLogout }: DashboardProps) {
  const navigate = useNavigate()
  const startWorkout = useWorkoutStore((s) => s.startWorkout)
  const activeWorkoutId = useWorkoutStore((s) => s.activeWorkoutId)
  const [startingWorkout, setStartingWorkout] = useState(false)

  const { data, isLoading, error } = useQuery<DashboardData>({
    queryKey: ['dashboard'],
    queryFn: () => api.getDashboard(),
    retry: 2,
  })

  const handleLogout = () => {
    api.logout()
    onLogout()
  }

  const handleStartWorkout = async () => {
    if (activeWorkoutId) {
      navigate('/workout/active')
      return
    }
    setStartingWorkout(true)
    try {
      const workout = await api.createWorkout()
      startWorkout(workout.id, workout.name)
      navigate('/workout/active')
    } catch {
      const tempId = `offline-${Date.now()}`
      startWorkout(tempId)
      navigate('/workout/active')
    } finally {
      setStartingWorkout(false)
    }
  }

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-text-secondary animate-pulse">Loading dashboard...</div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="min-h-screen flex items-center justify-center px-4">
        <div className="card max-w-md text-center">
          <div className="text-danger mb-3">Failed to load dashboard</div>
          <p className="text-text-muted text-sm mb-4">
            {error instanceof Error ? error.message : 'Something went wrong'}
          </p>
          <div className="flex gap-3 justify-center">
            <button onClick={() => window.location.reload()} className="btn-primary text-sm">
              Retry
            </button>
            <button onClick={handleLogout} className="btn-secondary text-sm">
              Log Out
            </button>
          </div>
        </div>
      </div>
    )
  }

  return (
    <div className="min-h-screen pb-20">
      {/* Header */}
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-4xl mx-auto px-4 py-4 flex justify-between items-center">
          <h1 className="text-2xl font-bold">FORGE</h1>
          <button
            onClick={() => navigate('/settings')}
            className="tap-target p-2 text-text-secondary hover:text-primary transition-colors"
          >
            <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
              <path d="M12 15a3 3 0 100-6 3 3 0 000 6z" />
              <path d="M19.4 15a1.65 1.65 0 00.33 1.82l.06.06a2 2 0 01-2.83 2.83l-.06-.06a1.65 1.65 0 00-1.82-.33 1.65 1.65 0 00-1 1.51V21a2 2 0 01-4 0v-.09A1.65 1.65 0 009 19.4a1.65 1.65 0 00-1.82.33l-.06.06a2 2 0 01-2.83-2.83l.06-.06A1.65 1.65 0 004.68 15a1.65 1.65 0 00-1.51-1H3a2 2 0 010-4h.09A1.65 1.65 0 004.6 9a1.65 1.65 0 00-.33-1.82l-.06-.06a2 2 0 012.83-2.83l.06.06A1.65 1.65 0 009 4.68a1.65 1.65 0 001-1.51V3a2 2 0 014 0v.09a1.65 1.65 0 001 1.51 1.65 1.65 0 001.82-.33l.06-.06a2 2 0 012.83 2.83l-.06.06A1.65 1.65 0 0019.4 9a1.65 1.65 0 001.51 1H21a2 2 0 010 4h-.09a1.65 1.65 0 00-1.51 1z" />
            </svg>
          </button>
        </div>
      </div>

      <div className="max-w-4xl mx-auto px-4 py-6 space-y-6">
        {/* Quick Actions */}
        <div className="grid grid-cols-2 gap-3">
          <button
            onClick={handleStartWorkout}
            disabled={startingWorkout}
            className="btn-primary text-base py-4"
          >
            {activeWorkoutId
              ? 'Resume Workout'
              : startingWorkout
              ? 'Starting...'
              : 'Start Workout'}
          </button>
          <button
            onClick={() => navigate('/nutrition')}
            className="btn-secondary text-base py-4"
          >
            Log Food
          </button>
        </div>

        {/* Macro Rings */}
        <section>
          <h2 className="mb-4">Today's Nutrition</h2>
          <div className="card">
            <div className="grid grid-cols-2 sm:grid-cols-4 gap-6">
              <MacroRing
                label="Calories"
                consumed={data?.today.nutrition.calories.consumed || 0}
                target={data?.today.nutrition.calories.target || 2400}
                color="#6C5CE7"
              />
              <MacroRing
                label="Protein"
                consumed={data?.today.nutrition.protein_g.consumed || 0}
                target={data?.today.nutrition.protein_g.target || 176}
                color="#00D68F"
              />
              <MacroRing
                label="Carbs"
                consumed={data?.today.nutrition.carbs_g.consumed || 0}
                target={data?.today.nutrition.carbs_g.target || 280}
                color="#FFB800"
              />
              <MacroRing
                label="Fat"
                consumed={data?.today.nutrition.fat_g.consumed || 0}
                target={data?.today.nutrition.fat_g.target || 80}
                color="#FF5252"
              />
            </div>
          </div>
        </section>

        {/* Today's Workout */}
        <section>
          <h2 className="mb-4">Workout</h2>
          <div className="card">
            {data?.today.workout ? (
              <div>
                <h3 className="text-primary mb-3">{data.today.workout.name}</h3>
                <div className="grid grid-cols-2 gap-4 text-sm">
                  <div>
                    <div className="text-text-muted">Duration</div>
                    <div className="text-xl font-mono">
                      {Math.floor((data.today.workout.duration_seconds || 0) / 60)} min
                    </div>
                  </div>
                  <div>
                    <div className="text-text-muted">Volume</div>
                    <div className="text-xl font-mono">
                      {data.today.workout.total_volume_kg >= 1000
                        ? `${(data.today.workout.total_volume_kg / 1000).toFixed(1)}t`
                        : `${Math.round(data.today.workout.total_volume_kg)}kg`}
                    </div>
                  </div>
                  <div>
                    <div className="text-text-muted">Exercises</div>
                    <div className="text-xl font-mono">{data.today.workout.exercises_count}</div>
                  </div>
                  <div>
                    <div className="text-text-muted">PRs</div>
                    <div className="text-xl font-mono text-celebration">
                      {data.today.workout.prs_count}
                    </div>
                  </div>
                </div>
              </div>
            ) : (
              <div className="text-center py-6">
                <p className="text-text-muted mb-3">No workout today</p>
                <button
                  onClick={handleStartWorkout}
                  disabled={startingWorkout}
                  className="text-primary text-sm font-medium hover:text-primary/80"
                >
                  {activeWorkoutId ? 'Resume your workout' : 'Start one now'}
                </button>
              </div>
            )}
          </div>
        </section>

        {/* Week Overview */}
        <section>
          <h2 className="mb-4">This Week</h2>
          <div className="card">
            <div className="flex justify-center gap-2 mb-4">
              {['M', 'T', 'W', 'T', 'F', 'S', 'S'].map((day, i) => (
                <div key={i} className="flex flex-col items-center">
                  <div className="text-xs text-text-muted mb-2">{day}</div>
                  <div
                    className={`w-10 h-10 rounded-full flex items-center justify-center text-sm ${
                      data?.week.workout_days[i]
                        ? 'bg-primary text-white'
                        : 'bg-surface-elevated text-text-muted'
                    }`}
                  >
                    {data?.week.workout_days[i] ? '✓' : ''}
                  </div>
                </div>
              ))}
            </div>
            {data?.week.current_streak ? (
              <div className="text-center">
                <div className="text-xl font-bold text-primary">
                  {data.week.current_streak} day streak
                </div>
              </div>
            ) : null}
          </div>
        </section>
      </div>
    </div>
  )
}
