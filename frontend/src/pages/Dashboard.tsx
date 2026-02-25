import { useQuery } from '@tanstack/react-query'
import { api } from '@/services/api'
import { DashboardData } from '@/types'

interface DashboardProps {
  onLogout: () => void
}

export default function Dashboard({ onLogout }: DashboardProps) {
  const { data, isLoading, error, isRefetching } = useQuery<DashboardData>({
    queryKey: ['dashboard'],
    queryFn: () => api.getDashboard(),
    staleTime: 30_000,        // Data considered fresh for 30 seconds
    refetchInterval: 60_000,  // Auto-refresh every 60 seconds
  })

  const handleLogout = () => {
    api.logout()
    onLogout()
  }

  if (isLoading) {
    return (
      <div className="min-h-screen pb-12">
        {/* Header */}
        <div className="bg-surface border-b border-text-muted">
          <div className="max-w-4xl mx-auto px-4 py-4 flex justify-between items-center">
            <h1 className="text-2xl font-bold">FORGE</h1>
            <button onClick={handleLogout} className="text-text-secondary hover:text-primary">
              Log Out
            </button>
          </div>
        </div>
        <div className="max-w-4xl mx-auto px-4 py-8 space-y-8">
          {/* Skeleton for nutrition */}
          <section>
            <div className="h-6 w-40 bg-surface-elevated rounded animate-pulse mb-6" />
            <div className="card">
              <div className="grid grid-cols-2 md:grid-cols-4 gap-8 mb-8">
                {[1, 2, 3, 4].map((i) => (
                  <div key={i} className="flex flex-col items-center">
                    <div className="w-28 h-28 rounded-full bg-surface-elevated animate-pulse" />
                    <div className="h-4 w-16 bg-surface-elevated rounded animate-pulse mt-2" />
                  </div>
                ))}
              </div>
            </div>
          </section>
          {/* Skeleton for workout */}
          <section>
            <div className="h-6 w-24 bg-surface-elevated rounded animate-pulse mb-6" />
            <div className="card">
              <div className="h-32 bg-surface-elevated rounded animate-pulse" />
            </div>
          </section>
          {/* Skeleton for week */}
          <section>
            <div className="h-6 w-28 bg-surface-elevated rounded animate-pulse mb-6" />
            <div className="card">
              <div className="flex justify-center gap-2 mb-6">
                {[1, 2, 3, 4, 5, 6, 7].map((i) => (
                  <div key={i} className="w-10 h-10 rounded-full bg-surface-elevated animate-pulse" />
                ))}
              </div>
            </div>
          </section>
        </div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="card max-w-md">
          <h2 className="text-danger mb-4">Error loading dashboard</h2>
          <p className="text-text-secondary mb-6">
            {error instanceof Error ? error.message : 'An error occurred'}
          </p>
          <button onClick={handleLogout} className="btn-primary">
            Log Out
          </button>
        </div>
      </div>
    )
  }

  const formatVolume = (kg: number): string => {
    if (kg >= 1000) {
      return `${(kg / 1000).toFixed(1)} tons`
    }
    return `${Math.round(kg)} kg`
  }

  const MacroRing = ({
    label,
    consumed,
    target,
    color
  }: {
    label: string
    consumed: number
    target: number
    color: string
  }) => {
    const percentage = target > 0 ? Math.min((consumed / target) * 100, 100) : 0
    const circumference = 2 * Math.PI * 45
    const strokeDashoffset = circumference - (percentage / 100) * circumference

    return (
      <div className="flex flex-col items-center">
        <div className="relative w-28 h-28">
          <svg className="w-full h-full transform -rotate-90">
            <circle
              cx="56"
              cy="56"
              r="45"
              stroke="currentColor"
              strokeWidth="8"
              fill="none"
              className="text-surface-elevated"
            />
            <circle
              cx="56"
              cy="56"
              r="45"
              stroke={color}
              strokeWidth="8"
              fill="none"
              strokeDasharray={circumference}
              strokeDashoffset={strokeDashoffset}
              strokeLinecap="round"
              className="transition-all duration-800 ease-out"
            />
          </svg>
          <div className="absolute inset-0 flex flex-col items-center justify-center">
            <div className="text-xl font-bold font-mono">{Math.round(consumed)}</div>
            <div className="text-xs text-text-muted">/ {Math.round(target)}</div>
          </div>
        </div>
        <div className="text-sm text-text-secondary mt-2">{label}</div>
      </div>
    )
  }

  return (
    <div className="min-h-screen pb-12">
      {/* Header */}
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-4xl mx-auto px-4 py-4 flex justify-between items-center">
          <div className="flex items-center gap-3">
            <h1 className="text-2xl font-bold">FORGE</h1>
            {isRefetching && (
              <div className="w-2 h-2 rounded-full bg-primary animate-pulse" />
            )}
          </div>
          <button onClick={handleLogout} className="text-text-secondary hover:text-primary">
            Log Out
          </button>
        </div>
      </div>

      <div className="max-w-4xl mx-auto px-4 py-8 space-y-8">
        {/* Onboarding prompt if no profile */}
        {data && !data.has_profile && (
          <section>
            <div className="card border border-primary/30 bg-primary/5">
              <div className="flex flex-col sm:flex-row items-start sm:items-center gap-4">
                <div className="flex-1">
                  <h3 className="text-primary font-semibold mb-1">Complete your profile</h3>
                  <p className="text-text-secondary text-sm">
                    Set up your profile to get personalized calorie and macro targets
                    based on your body stats and fitness goals.
                  </p>
                </div>
                <button className="btn-primary whitespace-nowrap">
                  Set Up Profile
                </button>
              </div>
            </div>
          </section>
        )}

        {/* Today's Summary */}
        <section>
          <h2 className="mb-6">Today's Progress</h2>

          {/* Macro Rings */}
          <div className="card">
            <div className="grid grid-cols-2 md:grid-cols-4 gap-8 mb-8">
              <MacroRing
                label="Calories"
                consumed={data?.today.nutrition.calories.consumed ?? 0}
                target={data?.today.nutrition.calories.target ?? 2400}
                color="#6C5CE7"
              />
              <MacroRing
                label="Protein"
                consumed={data?.today.nutrition.protein_g.consumed ?? 0}
                target={data?.today.nutrition.protein_g.target ?? 176}
                color="#00D68F"
              />
              <MacroRing
                label="Carbs"
                consumed={data?.today.nutrition.carbs_g.consumed ?? 0}
                target={data?.today.nutrition.carbs_g.target ?? 280}
                color="#FFB800"
              />
              <MacroRing
                label="Fat"
                consumed={data?.today.nutrition.fat_g.consumed ?? 0}
                target={data?.today.nutrition.fat_g.target ?? 80}
                color="#FF5252"
              />
            </div>

            <button className="btn-primary w-full">
              Log Food
            </button>
          </div>
        </section>

        {/* Today's Workout */}
        <section>
          <h2 className="mb-6">Workout</h2>
          <div className="card">
            {data?.today.workout ? (
              <div>
                <h3 className="text-primary mb-4">{data.today.workout.name}</h3>
                <div className="grid grid-cols-2 gap-4 text-sm">
                  <div>
                    <div className="text-text-muted">Duration</div>
                    <div className="text-xl font-mono">
                      {Math.floor((data.today.workout.duration_seconds || 0) / 60)} min
                    </div>
                  </div>
                  <div>
                    <div className="text-text-muted">Total Volume</div>
                    <div className="text-xl font-mono">
                      {formatVolume(data.today.workout.total_volume_kg)}
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
              <div className="text-center py-8">
                <p className="text-text-secondary mb-4">No workout today</p>
                <button className="btn-primary">
                  Start Workout
                </button>
              </div>
            )}
          </div>
        </section>

        {/* Week Overview */}
        <section>
          <h2 className="mb-6">This Week</h2>
          <div className="card">
            <div className="flex justify-center gap-2 mb-6">
              {['M', 'T', 'W', 'T', 'F', 'S', 'S'].map((day, i) => (
                <div key={i} className="flex flex-col items-center">
                  <div className="text-xs text-text-muted mb-2">{day}</div>
                  <div
                    className={`w-10 h-10 rounded-full flex items-center justify-center ${
                      data?.week.workout_days[i]
                        ? 'bg-primary text-white'
                        : 'bg-surface-elevated'
                    }`}
                  >
                    {data?.week.workout_days[i] && (
                      <span className="text-sm font-bold">&#10003;</span>
                    )}
                  </div>
                </div>
              ))}
            </div>
            {(data?.week.current_streak ?? 0) > 0 && (
              <div className="text-center">
                <div className="text-2xl font-bold text-primary">
                  {data!.week.current_streak} day streak
                </div>
              </div>
            )}
          </div>
        </section>
      </div>
    </div>
  )
}
