import { useState, useEffect, useMemo } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type { WeightData, WeightTrend } from '@/types'

interface ChartScaling {
  minWeight: number
  maxWeight: number
}

type TimeRange = '1w' | '1m' | '3m' | '6m' | '1y'

function getTimeRangeDays(range: TimeRange): number {
  switch (range) {
    case '1w': return 7
    case '1m': return 30
    case '3m': return 90
    case '6m': return 180
    case '1y': return 365
  }
}

function filterByTimeRange(data: WeightData[], range: TimeRange): WeightData[] {
  const days = getTimeRangeDays(range)
  const cutoff = new Date()
  cutoff.setDate(cutoff.getDate() - days)
  return data.filter(d => new Date(d.logged_at) >= cutoff)
}

export default function WeightTracker() {
  const navigate = useNavigate()
  const [weightInput, setWeightInput] = useState('')
  const [isLogging, setIsLogging] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const [success, setSuccess] = useState<string | null>(null)
  const [deleteError, setDeleteError] = useState<string | null>(null)

  const [allWeightHistory, setAllWeightHistory] = useState<WeightData[]>([])
  const [trend, setTrend] = useState<WeightTrend | null>(null)
  const [loading, setLoading] = useState(true)
  const [timeRange, setTimeRange] = useState<TimeRange>('3m')
  const [deletingId, setDeletingId] = useState<string | null>(null)

  const [newBadges, setNewBadges] = useState<{ code: string; name: string }[]>([])

  const weightHistory = useMemo(
    () => filterByTimeRange(allWeightHistory, timeRange),
    [allWeightHistory, timeRange]
  )

  useEffect(() => {
    loadData()
  }, [])

  const loadData = async () => {
    setLoading(true)
    try {
      const [historyRes, trendRes] = await Promise.all([
        api.getWeightHistory(),
        api.getWeightTrend()
      ])
      setAllWeightHistory(historyRes.data)
      setTrend(trendRes)
    } catch (err) {
      console.error('Failed to load weight data:', err)
    } finally {
      setLoading(false)
    }
  }

  const logWeight = async () => {
    const weight = parseFloat(weightInput)
    if (isNaN(weight) || weight < 30 || weight > 300) {
      setError('Please enter a valid weight between 30-300 kg')
      return
    }

    setIsLogging(true)
    setError(null)

    try {
      const result = await api.logWeight(weight)
      setSuccess('Weight logged successfully!')
      setWeightInput('')

      if (result.new_badges && result.new_badges.length > 0) {
        setNewBadges(result.new_badges)
      }

      loadData()
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to log weight')
    } finally {
      setIsLogging(false)
    }
  }

  const deleteWeightEntry = async (id: string) => {
    setDeletingId(id)
    setDeleteError(null)
    try {
      await api.deleteWeight(id)
      setAllWeightHistory(prev => prev.filter(e => e.id !== id))
    } catch (err) {
      setDeleteError(err instanceof Error ? err.message : 'Failed to delete entry')
    } finally {
      setDeletingId(null)
    }
  }

  const getChartScaling = (): ChartScaling | null => {
    if (weightHistory.length < 2) return null

    const weights = weightHistory.map(d => d.weight_kg)
    const minWeight = Math.min(...weights) - 2
    const maxWeight = Math.max(...weights) + 2

    return { minWeight, maxWeight }
  }

  const chartScaling = getChartScaling()

  const scaleY = (weight: number): number => {
    if (!chartScaling) return 0
    const range = chartScaling.maxWeight - chartScaling.minWeight
    return 50 - ((weight - chartScaling.minWeight) / range) * 50
  }

  const getWeightDots = (): string => {
    if (weightHistory.length < 2) return ''
    return weightHistory
      .map((d, i) => {
        const x = (i / (weightHistory.length - 1)) * 100
        const y = scaleY(d.weight_kg)
        return `${x},${y}`
      })
      .join(' ')
  }

  const formatDate = (dateStr: string): string => {
    const date = new Date(dateStr)
    return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' })
  }

  const getTrendDirectionLabel = (): string => {
    if (!trend) return ''
    switch (trend.trend_direction) {
      case 'losing': return 'losing'
      case 'gaining': return 'gaining'
      case 'stable': return 'stable'
    }
  }

  return (
    <div className="min-h-screen bg-background pb-20">
      {/* Header */}
      <div className="bg-surface shadow-sm">
        <div className="max-w-lg mx-auto px-4 py-4 flex items-center justify-between">
          <button onClick={() => navigate(-1)} className="text-text-secondary">
            &larr; Back
          </button>
          <h1 className="text-lg font-semibold text-text-primary">Weight Tracker</h1>
          <div className="w-10" />
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-6 space-y-6">
        {/* Badge Celebration Modal */}
        {newBadges.length > 0 && (
          <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50">
            <div className="bg-surface rounded-2xl p-8 mx-4 text-center animate-bounce-in">
              <div className="text-6xl mb-4">&#127942;</div>
              <h2 className="text-2xl font-bold text-text-primary mb-2">Badge Earned!</h2>
              {newBadges.map(badge => (
                <div key={badge.code} className="text-lg text-text-secondary">
                  {badge.name}
                </div>
              ))}
              <button
                onClick={() => setNewBadges([])}
                className="mt-6 px-6 py-2 bg-primary text-white rounded-full"
              >
                Awesome!
              </button>
            </div>
          </div>
        )}

        {/* Log Weight Card */}
        <div className="bg-surface rounded-card shadow-sm p-6">
          <h2 className="text-lg font-medium text-text-primary mb-4">Log Today's Weight</h2>

          {error && (
            <div className="mb-4 p-3 bg-danger/10 border border-danger/30 rounded-lg text-danger text-sm">
              {error}
            </div>
          )}

          {success && (
            <div className="mb-4 p-3 bg-success/10 border border-success/30 rounded-lg text-success text-sm">
              {success}
            </div>
          )}

          <div className="flex gap-3">
            <div className="flex-1">
              <div className="relative">
                <input
                  type="number"
                  step="0.1"
                  placeholder="82.5"
                  value={weightInput}
                  onChange={e => {
                    setWeightInput(e.target.value)
                    setError(null)
                  }}
                  className="w-full px-4 py-4 text-2xl font-bold bg-surface-elevated border border-border rounded-card text-center text-text-primary placeholder:text-text-muted"
                />
                <span className="absolute right-4 top-1/2 -translate-y-1/2 text-text-muted">kg</span>
              </div>
            </div>
            <button
              onClick={logWeight}
              disabled={isLogging || !weightInput}
              className="px-8 py-4 bg-primary text-white rounded-card font-medium disabled:opacity-50"
            >
              {isLogging ? '...' : 'Log'}
            </button>
          </div>
        </div>

        {/* Trend Summary */}
        {trend && trend.data_points >= 3 && (
          <div className="bg-gradient-to-br from-primary to-primary/80 rounded-card shadow-lg p-6 text-white">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <div className="text-white/70 text-sm">Current Weight</div>
                <div className="text-3xl font-bold">
                  {trend.current_weight.toFixed(1)} kg
                </div>
              </div>
              <div>
                <div className="text-white/70 text-sm">Weekly Change</div>
                <div className={`text-3xl font-bold ${trend.avg_weekly_change < 0 ? 'text-success' : trend.avg_weekly_change > 0 ? 'text-danger' : ''}`}>
                  {trend.avg_weekly_change > 0 ? '+' : ''}{trend.avg_weekly_change.toFixed(2)} kg
                </div>
              </div>
            </div>

            <div className="mt-4 pt-4 border-t border-white/20">
              <div className="flex justify-between items-center">
                <div>
                  <span className="text-white/70">Total Change </span>
                  <span className="font-medium">
                    {trend.change_kg > 0 ? '+' : ''}{trend.change_kg.toFixed(1)} kg
                  </span>
                </div>
                <div className="text-xl font-bold">{trend.change_percent.toFixed(1)}%</div>
              </div>
              <div className="mt-2 text-sm text-white/70">
                Started at {trend.start_weight.toFixed(1)} kg &middot; Currently {getTrendDirectionLabel()} &middot; {trend.data_points} data points
              </div>
            </div>
          </div>
        )}

        {/* Weight Chart */}
        <div className="bg-surface rounded-card shadow-sm p-6">
          <div className="flex justify-between items-center mb-4">
            <h2 className="text-lg font-medium text-text-primary">Weight History</h2>
            <div className="flex gap-1">
              {(['1w', '1m', '3m', '6m', '1y'] as const).map(range => (
                <button
                  key={range}
                  onClick={() => setTimeRange(range)}
                  className={`px-3 py-1 text-sm rounded-full ${
                    timeRange === range
                      ? 'bg-primary text-white'
                      : 'bg-surface-elevated text-text-secondary'
                  }`}
                >
                  {range.toUpperCase()}
                </button>
              ))}
            </div>
          </div>

          {loading ? (
            <div className="h-48 flex items-center justify-center text-text-muted">
              Loading...
            </div>
          ) : weightHistory.length < 2 ? (
            <div className="h-48 flex items-center justify-center text-text-muted">
              Log at least 2 weigh-ins to see your chart
            </div>
          ) : chartScaling && (
            <div className="relative h-48">
              <svg viewBox="0 0 100 50" className="w-full h-full" preserveAspectRatio="none">
                {/* Grid lines */}
                <line x1="0" y1="12.5" x2="100" y2="12.5" stroke="currentColor" strokeWidth="0.5" className="text-border" opacity="0.3" />
                <line x1="0" y1="25" x2="100" y2="25" stroke="currentColor" strokeWidth="0.5" className="text-border" opacity="0.3" />
                <line x1="0" y1="37.5" x2="100" y2="37.5" stroke="currentColor" strokeWidth="0.5" className="text-border" opacity="0.3" />

                {/* Weight line */}
                <polyline
                  points={getWeightDots()}
                  fill="none"
                  stroke="currentColor"
                  strokeWidth="1.5"
                  className="text-primary"
                />

                {/* Weight dots */}
                {weightHistory.map((d, i) => {
                  const x = (i / (weightHistory.length - 1)) * 100
                  const y = scaleY(d.weight_kg)
                  return (
                    <circle
                      key={d.id}
                      cx={x}
                      cy={y}
                      r="1"
                      fill="currentColor"
                      className="text-text-muted"
                    />
                  )
                })}
              </svg>

              {/* Y-axis labels */}
              <div className="absolute top-0 left-0 h-full flex flex-col justify-between text-xs text-text-muted pointer-events-none">
                <span>{chartScaling.maxWeight.toFixed(0)}</span>
                <span>{chartScaling.minWeight.toFixed(0)}</span>
              </div>

              {/* Legend */}
              <div className="absolute bottom-0 right-0 flex gap-4 text-xs text-text-secondary">
                <div className="flex items-center gap-1">
                  <div className="w-2 h-2 rounded-full bg-text-muted" />
                  <span>Weight</span>
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Recent Weigh-ins */}
        <div className="bg-surface rounded-card shadow-sm p-6">
          <h2 className="text-lg font-medium text-text-primary mb-4">Recent Weigh-ins</h2>

          {deleteError && (
            <div className="mb-4 p-3 bg-danger/10 border border-danger/30 rounded-lg text-danger text-sm">
              {deleteError}
            </div>
          )}

          <div className="space-y-3">
            {weightHistory.slice(-10).reverse().map(entry => (
              <div key={entry.id} className="flex justify-between items-center py-2 border-b border-border last:border-b-0">
                <div>
                  <div className="text-text-secondary">{formatDate(entry.logged_at)}</div>
                  {entry.notes && (
                    <div className="text-xs text-text-muted mt-0.5">{entry.notes}</div>
                  )}
                </div>
                <div className="flex items-center gap-3">
                  <div className="font-medium text-text-primary">{entry.weight_kg.toFixed(1)} kg</div>
                  <button
                    onClick={() => deleteWeightEntry(entry.id)}
                    disabled={deletingId === entry.id}
                    className="p-1.5 text-text-muted hover:text-danger hover:bg-surface-elevated rounded-lg transition-colors disabled:opacity-50"
                    title="Delete entry"
                  >
                    {deletingId === entry.id ? (
                      <svg className="w-4 h-4 animate-spin" viewBox="0 0 24 24" fill="none">
                        <circle cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="2" opacity="0.3" />
                        <path d="M12 2a10 10 0 0 1 10 10" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
                      </svg>
                    ) : (
                      <svg className="w-4 h-4" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth="2">
                        <path strokeLinecap="round" strokeLinejoin="round" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
                      </svg>
                    )}
                  </button>
                </div>
              </div>
            ))}

            {weightHistory.length === 0 && !loading && (
              <div className="text-center text-text-muted py-4">
                No weigh-ins recorded yet
              </div>
            )}
          </div>
        </div>

        {/* Insights */}
        {trend && trend.data_points >= 3 && (
          <div className="bg-primary/10 rounded-card p-4">
            <h3 className="font-medium text-text-primary mb-2">Insights</h3>
            <p className="text-text-secondary text-sm">
              Your current weight is <strong>{trend.current_weight.toFixed(1)} kg</strong>,
              {' '}{trend.trend_direction === 'losing' ? 'down' : trend.trend_direction === 'gaining' ? 'up' : 'stable at'}{' '}
              <strong>{Math.abs(trend.change_kg).toFixed(1)} kg</strong> overall
              ({trend.change_percent > 0 ? '+' : ''}{trend.change_percent.toFixed(1)}%).
              {trend.avg_weekly_change !== 0 && (
                <>
                  {' '}Average weekly change: <strong>{trend.avg_weekly_change > 0 ? '+' : ''}{trend.avg_weekly_change.toFixed(2)} kg</strong>.
                </>
              )}
            </p>
          </div>
        )}
      </div>
    </div>
  )
}
