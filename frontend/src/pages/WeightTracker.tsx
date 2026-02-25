import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type { WeightData, WeightTrend } from '@/types'

export default function WeightTracker() {
  const navigate = useNavigate()
  const [weightInput, setWeightInput] = useState('')
  const [isLogging, setIsLogging] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const [success, setSuccess] = useState<string | null>(null)

  const [weightHistory, setWeightHistory] = useState<WeightData[]>([])
  const [trend, setTrend] = useState<WeightTrend | null>(null)
  const [loading, setLoading] = useState(true)
  const [timeRange, setTimeRange] = useState<'1w' | '1m' | '3m' | '6m' | '1y'>('3m')

  const [newBadges, setNewBadges] = useState<{ code: string; name: string }[]>([])

  useEffect(() => {
    loadData()
  }, [timeRange])

  const loadData = async () => {
    setLoading(true)
    try {
      const [historyRes, trendRes] = await Promise.all([
        api.getWeightHistory(),
        api.getWeightTrend()
      ])
      setWeightHistory(historyRes.data)
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

  const getChartPath = () => {
    if (weightHistory.length < 2) return ''

    // Find min/max for scaling
    const weights = weightHistory.map(d => d.scale_weight_kg)
    const trendWeights = weightHistory
      .filter(d => d.trend_weight_kg)
      .map(d => d.trend_weight_kg!)

    const allWeights = [...weights, ...trendWeights]
    const minWeight = Math.min(...allWeights) - 2
    const maxWeight = Math.max(...allWeights) + 2
    const range = maxWeight - minWeight

    const width = 100
    const height = 50

    const scaleX = (i: number) => (i / (weightHistory.length - 1)) * width
    const scaleY = (w: number) => height - ((w - minWeight) / range) * height

    // Scale weight line (dots)
    const scaleDots = weightHistory
      .map((d, i) => `${scaleX(i)},${scaleY(d.scale_weight_kg)}`)
      .join(' ')

    // Trend line
    const trendPoints = weightHistory
      .filter(d => d.trend_weight_kg)
      .map((d, i) => `${scaleX(weightHistory.indexOf(d))},${scaleY(d.trend_weight_kg!)}`)
      .join(' ')

    return { scaleDots, trendPoints, minWeight, maxWeight }
  }

  const chartData = getChartPath()

  const getRateColor = (rate: number) => {
    if (!trend) return 'text-gray-600'
    // Assume goal is to lose weight if trend is down
    const isTowardsGoal = (trend.current_trend_weight_kg > trend.goal_weight_kg && rate < 0) ||
                          (trend.current_trend_weight_kg < trend.goal_weight_kg && rate > 0)
    return isTowardsGoal ? 'text-green-600' : 'text-red-600'
  }

  return (
    <div className="min-h-screen bg-gray-50 pb-20">
      {/* Header */}
      <div className="bg-white shadow-sm">
        <div className="max-w-lg mx-auto px-4 py-4 flex items-center justify-between">
          <button onClick={() => navigate(-1)} className="text-gray-600">
            ← Back
          </button>
          <h1 className="text-lg font-semibold">Weight Tracker</h1>
          <div className="w-10" />
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-6 space-y-6">
        {/* Badge Celebration Modal */}
        {newBadges.length > 0 && (
          <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50">
            <div className="bg-white rounded-2xl p-8 mx-4 text-center animate-bounce-in">
              <div className="text-6xl mb-4">🏆</div>
              <h2 className="text-2xl font-bold mb-2">Badge Earned!</h2>
              {newBadges.map(badge => (
                <div key={badge.code} className="text-lg text-gray-600">
                  {badge.name}
                </div>
              ))}
              <button
                onClick={() => setNewBadges([])}
                className="mt-6 px-6 py-2 bg-blue-600 text-white rounded-full"
              >
                Awesome!
              </button>
            </div>
          </div>
        )}

        {/* Log Weight Card */}
        <div className="bg-white rounded-xl shadow-sm p-6">
          <h2 className="text-lg font-medium mb-4">Log Today's Weight</h2>

          {error && (
            <div className="mb-4 p-3 bg-red-50 border border-red-200 rounded-lg text-red-700 text-sm">
              {error}
            </div>
          )}

          {success && (
            <div className="mb-4 p-3 bg-green-50 border border-green-200 rounded-lg text-green-700 text-sm">
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
                  onChange={e => setWeightInput(e.target.value)}
                  className="w-full px-4 py-4 text-2xl font-bold border rounded-xl text-center"
                />
                <span className="absolute right-4 top-1/2 -translate-y-1/2 text-gray-400">kg</span>
              </div>
            </div>
            <button
              onClick={logWeight}
              disabled={isLogging || !weightInput}
              className="px-8 py-4 bg-blue-600 text-white rounded-xl font-medium disabled:opacity-50"
            >
              {isLogging ? '...' : 'Log'}
            </button>
          </div>
        </div>

        {/* Trend Summary */}
        {trend && trend.is_reliable && (
          <div className="bg-gradient-to-br from-blue-500 to-blue-600 rounded-xl shadow-lg p-6 text-white">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <div className="text-blue-100 text-sm">Trend Weight</div>
                <div className="text-3xl font-bold">
                  {trend.current_trend_weight_kg.toFixed(1)} kg
                </div>
              </div>
              <div>
                <div className="text-blue-100 text-sm">Weekly Change</div>
                <div className={`text-3xl font-bold ${trend.weekly_rate_kg < 0 ? 'text-green-300' : 'text-red-300'}`}>
                  {trend.weekly_rate_kg > 0 ? '+' : ''}{trend.weekly_rate_kg.toFixed(2)} kg
                </div>
              </div>
            </div>

            <div className="mt-4 pt-4 border-t border-blue-400">
              <div className="flex justify-between items-center">
                <div>
                  <span className="text-blue-100">Progress to </span>
                  <span className="font-medium">{trend.goal_weight_kg} kg</span>
                </div>
                <div className="text-xl font-bold">{Math.round(trend.progress_percent)}%</div>
              </div>
              <div className="mt-2 h-2 bg-blue-400 rounded-full overflow-hidden">
                <div
                  className="h-full bg-white rounded-full transition-all"
                  style={{ width: `${Math.min(100, Math.max(0, trend.progress_percent))}%` }}
                />
              </div>
              {trend.estimated_days_to_goal && (
                <div className="mt-2 text-sm text-blue-100">
                  ~{Math.round(trend.estimated_days_to_goal / 7)} weeks to goal at current rate
                </div>
              )}
            </div>
          </div>
        )}

        {/* Weight Chart */}
        <div className="bg-white rounded-xl shadow-sm p-6">
          <div className="flex justify-between items-center mb-4">
            <h2 className="text-lg font-medium">Weight History</h2>
            <div className="flex gap-1">
              {(['1w', '1m', '3m', '6m', '1y'] as const).map(range => (
                <button
                  key={range}
                  onClick={() => setTimeRange(range)}
                  className={`px-3 py-1 text-sm rounded-full ${
                    timeRange === range
                      ? 'bg-blue-600 text-white'
                      : 'bg-gray-100 text-gray-600'
                  }`}
                >
                  {range.toUpperCase()}
                </button>
              ))}
            </div>
          </div>

          {loading ? (
            <div className="h-48 flex items-center justify-center text-gray-400">
              Loading...
            </div>
          ) : weightHistory.length < 2 ? (
            <div className="h-48 flex items-center justify-center text-gray-400">
              Log at least 2 weigh-ins to see your chart
            </div>
          ) : (
            <div className="relative h-48">
              <svg viewBox="0 0 100 50" className="w-full h-full" preserveAspectRatio="none">
                {/* Grid lines */}
                <line x1="0" y1="12.5" x2="100" y2="12.5" stroke="#f0f0f0" strokeWidth="0.5" />
                <line x1="0" y1="25" x2="100" y2="25" stroke="#f0f0f0" strokeWidth="0.5" />
                <line x1="0" y1="37.5" x2="100" y2="37.5" stroke="#f0f0f0" strokeWidth="0.5" />

                {/* Goal line */}
                {trend && (
                  <line
                    x1="0"
                    y1={50 - ((trend.goal_weight_kg - (chartData as any).minWeight) / ((chartData as any).maxWeight - (chartData as any).minWeight)) * 50}
                    x2="100"
                    y2={50 - ((trend.goal_weight_kg - (chartData as any).minWeight) / ((chartData as any).maxWeight - (chartData as any).minWeight)) * 50}
                    stroke="#10b981"
                    strokeWidth="0.5"
                    strokeDasharray="2,2"
                  />
                )}

                {/* Trend line */}
                {(chartData as any).trendPoints && (
                  <polyline
                    points={(chartData as any).trendPoints}
                    fill="none"
                    stroke="#3b82f6"
                    strokeWidth="1.5"
                  />
                )}

                {/* Scale weight dots */}
                {weightHistory.map((d, i) => {
                  const x = (i / (weightHistory.length - 1)) * 100
                  const y = 50 - ((d.scale_weight_kg - (chartData as any).minWeight) / ((chartData as any).maxWeight - (chartData as any).minWeight)) * 50
                  return (
                    <circle
                      key={i}
                      cx={x}
                      cy={y}
                      r="1"
                      fill="#9ca3af"
                    />
                  )
                })}
              </svg>

              {/* Legend */}
              <div className="absolute bottom-0 right-0 flex gap-4 text-xs">
                <div className="flex items-center gap-1">
                  <div className="w-2 h-2 rounded-full bg-gray-400" />
                  <span>Scale</span>
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-4 h-0.5 bg-blue-500" />
                  <span>Trend</span>
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-4 h-0.5 border-t border-dashed border-green-500" />
                  <span>Goal</span>
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Recent Weigh-ins */}
        <div className="bg-white rounded-xl shadow-sm p-6">
          <h2 className="text-lg font-medium mb-4">Recent Weigh-ins</h2>
          <div className="space-y-3">
            {weightHistory.slice(-7).reverse().map((entry, i) => (
              <div key={i} className="flex justify-between items-center py-2 border-b last:border-b-0">
                <div className="text-gray-600">{entry.date}</div>
                <div className="flex gap-4">
                  <div className="text-gray-400">{entry.scale_weight_kg.toFixed(1)} kg</div>
                  {entry.trend_weight_kg && (
                    <div className="font-medium text-blue-600">
                      {entry.trend_weight_kg.toFixed(1)} kg
                    </div>
                  )}
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Insights */}
        {trend && trend.is_reliable && (
          <div className="bg-blue-50 rounded-xl p-4">
            <h3 className="font-medium text-blue-900 mb-2">Insights</h3>
            <p className="text-blue-800 text-sm">
              Your trend weight is <strong>{trend.current_trend_weight_kg.toFixed(1)} kg</strong>,
              {' '}{trend.weekly_rate_kg < 0 ? 'down' : 'up'}{' '}
              <strong>{Math.abs(trend.weekly_rate_kg).toFixed(2)} kg</strong> this week.
              {trend.estimated_days_to_goal && (
                <>
                  {' '}At this rate, you'll reach <strong>{trend.goal_weight_kg} kg</strong> in
                  approximately <strong>{Math.round(trend.estimated_days_to_goal / 7)} weeks</strong>.
                </>
              )}
            </p>
          </div>
        )}
      </div>
    </div>
  )
}
