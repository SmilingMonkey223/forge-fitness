import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type {
  MuscleGroupVolume,
  PersonalRecord,
  ConsistencyPoint,
  Streaks,
  CheckInRecommendation
} from '@/types'

type Tab = 'training' | 'nutrition'

export default function Analytics() {
  const navigate = useNavigate()
  const [activeTab, setActiveTab] = useState<Tab>('training')
  const [loading, setLoading] = useState(true)

  // Training data
  const [muscleDistribution, setMuscleDistribution] = useState<MuscleGroupVolume[]>([])
  const [prs, setPrs] = useState<PersonalRecord[]>([])
  const [consistency, setConsistency] = useState<ConsistencyPoint[]>([])
  const [streaks, setStreaks] = useState<Streaks | null>(null)

  // Nutrition data
  const [checkin, setCheckin] = useState<CheckInRecommendation | null>(null)
  const [checkinLoading, setCheckinLoading] = useState(false)
  const [checkinError, setCheckinError] = useState<string | null>(null)

  useEffect(() => {
    loadData()
  }, [activeTab])

  const loadData = async () => {
    setLoading(true)
    try {
      if (activeTab === 'training') {
        const [muscleRes, prRes, consistencyRes, streaksRes] = await Promise.all([
          api.getMuscleDistribution(),
          api.getPRHistory(),
          api.getTrainingConsistency(),
          api.getStreaks()
        ])
        setMuscleDistribution(muscleRes.data)
        setPrs(prRes.prs.slice(0, 10))
        setConsistency(consistencyRes.data)
        setStreaks(streaksRes)
      }
    } catch (err) {
      console.error('Failed to load analytics:', err)
    } finally {
      setLoading(false)
    }
  }

  const generateCheckin = async () => {
    setCheckinLoading(true)
    setCheckinError(null)
    try {
      const result = await api.generateNutritionCheckin()
      setCheckin(result)
    } catch (err) {
      setCheckinError(err instanceof Error ? err.message : 'Failed to generate check-in')
    } finally {
      setCheckinLoading(false)
    }
  }

  const getMuscleColor = (index: number) => {
    const colors = [
      'bg-blue-500', 'bg-green-500', 'bg-purple-500', 'bg-orange-500',
      'bg-pink-500', 'bg-cyan-500', 'bg-yellow-500', 'bg-red-500'
    ]
    return colors[index % colors.length]
  }

  const getConsistencyColor = (point: ConsistencyPoint) => {
    if (!point.workout_completed) return 'bg-gray-100'
    if (point.volume_kg > 5000) return 'bg-green-600'
    if (point.volume_kg > 2500) return 'bg-green-400'
    return 'bg-green-200'
  }

  return (
    <div className="min-h-screen bg-gray-50 pb-20">
      {/* Header */}
      <div className="bg-white shadow-sm">
        <div className="max-w-lg mx-auto px-4 py-4 flex items-center justify-between">
          <button onClick={() => navigate(-1)} className="text-gray-600">
            ← Back
          </button>
          <h1 className="text-lg font-semibold">Analytics</h1>
          <div className="w-10" />
        </div>

        {/* Tabs */}
        <div className="max-w-lg mx-auto px-4">
          <div className="flex border-b">
            <button
              onClick={() => setActiveTab('training')}
              className={`flex-1 py-3 text-center font-medium border-b-2 transition-colors ${
                activeTab === 'training'
                  ? 'border-blue-600 text-blue-600'
                  : 'border-transparent text-gray-500'
              }`}
            >
              Training
            </button>
            <button
              onClick={() => setActiveTab('nutrition')}
              className={`flex-1 py-3 text-center font-medium border-b-2 transition-colors ${
                activeTab === 'nutrition'
                  ? 'border-blue-600 text-blue-600'
                  : 'border-transparent text-gray-500'
              }`}
            >
              Nutrition
            </button>
          </div>
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-6 space-y-6">
        {loading ? (
          <div className="flex items-center justify-center h-48 text-gray-400">
            Loading...
          </div>
        ) : activeTab === 'training' ? (
          <>
            {/* Streak Cards */}
            {streaks && (
              <div className="grid grid-cols-3 gap-3">
                <div className="bg-white rounded-xl shadow-sm p-4 text-center">
                  <div className="text-3xl font-bold text-orange-500">
                    {streaks.workout.current}
                  </div>
                  <div className="text-xs text-gray-500 mt-1">Workout Streak</div>
                  <div className="text-xs text-gray-400">Best: {streaks.workout.best}</div>
                </div>
                <div className="bg-white rounded-xl shadow-sm p-4 text-center">
                  <div className="text-3xl font-bold text-green-500">
                    {streaks.logging.current}
                  </div>
                  <div className="text-xs text-gray-500 mt-1">Logging Streak</div>
                  <div className="text-xs text-gray-400">Best: {streaks.logging.best}</div>
                </div>
                <div className="bg-white rounded-xl shadow-sm p-4 text-center">
                  <div className="text-3xl font-bold text-blue-500">
                    {streaks.weight.current}
                  </div>
                  <div className="text-xs text-gray-500 mt-1">Weight Streak</div>
                  <div className="text-xs text-gray-400">Best: {streaks.weight.best}</div>
                </div>
              </div>
            )}

            {/* Consistency Heatmap */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Training Consistency</h2>
              <div className="grid grid-cols-7 gap-1">
                {['S', 'M', 'T', 'W', 'T', 'F', 'S'].map((day, i) => (
                  <div key={i} className="text-center text-xs text-gray-400 mb-1">
                    {day}
                  </div>
                ))}
                {consistency.slice(-28).map((point, i) => (
                  <div
                    key={i}
                    className={`aspect-square rounded-sm ${getConsistencyColor(point)}`}
                    title={`${point.date}: ${point.workout_completed ? `${point.volume_kg}kg` : 'Rest'}`}
                  />
                ))}
              </div>
              <div className="mt-4 flex justify-end gap-2 text-xs text-gray-500">
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-gray-100" /> Rest
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-green-200" /> Light
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-green-400" /> Medium
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-green-600" /> Heavy
                </div>
              </div>
            </div>

            {/* Muscle Distribution */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Muscle Distribution (4 weeks)</h2>
              {muscleDistribution.length === 0 ? (
                <p className="text-gray-400 text-center py-8">
                  Complete some workouts to see your muscle distribution
                </p>
              ) : (
                <>
                  {/* Donut Chart */}
                  <div className="relative w-48 h-48 mx-auto mb-6">
                    <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
                      {muscleDistribution.reduce((acc, group, i) => {
                        const startAngle = acc.offset
                        const angle = (group.percentage / 100) * 360
                        const endAngle = startAngle + angle

                        const startRad = (startAngle * Math.PI) / 180
                        const endRad = (endAngle * Math.PI) / 180

                        const x1 = 50 + 40 * Math.cos(startRad)
                        const y1 = 50 + 40 * Math.sin(startRad)
                        const x2 = 50 + 40 * Math.cos(endRad)
                        const y2 = 50 + 40 * Math.sin(endRad)

                        const largeArc = angle > 180 ? 1 : 0

                        acc.paths.push(
                          <path
                            key={i}
                            d={`M 50 50 L ${x1} ${y1} A 40 40 0 ${largeArc} 1 ${x2} ${y2} Z`}
                            className={getMuscleColor(i)}
                            fill="currentColor"
                          />
                        )
                        acc.offset = endAngle
                        return acc
                      }, { paths: [] as JSX.Element[], offset: 0 }).paths}
                    </svg>
                    <div className="absolute inset-0 flex items-center justify-center">
                      <div className="w-24 h-24 bg-white rounded-full flex items-center justify-center">
                        <span className="text-lg font-bold text-gray-700">
                          {muscleDistribution.reduce((sum, g) => sum + g.total_sets, 0)} sets
                        </span>
                      </div>
                    </div>
                  </div>

                  {/* Legend */}
                  <div className="grid grid-cols-2 gap-2">
                    {muscleDistribution.map((group, i) => (
                      <div key={group.muscle_group} className="flex items-center gap-2">
                        <div className={`w-3 h-3 rounded-full ${getMuscleColor(i)}`} />
                        <span className="text-sm text-gray-600 flex-1 capitalize">
                          {group.muscle_group.replace('_', ' ')}
                        </span>
                        <span className="text-sm font-medium">{group.total_sets}</span>
                      </div>
                    ))}
                  </div>
                </>
              )}
            </div>

            {/* Recent PRs */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Recent PRs</h2>
              {prs.length === 0 ? (
                <p className="text-gray-400 text-center py-8">
                  No personal records yet. Keep training!
                </p>
              ) : (
                <div className="space-y-3">
                  {prs.map(pr => (
                    <div key={pr.id} className="flex items-center gap-3 py-2 border-b last:border-b-0">
                      <div className="text-2xl">🏆</div>
                      <div className="flex-1">
                        <div className="font-medium">{pr.exercise_name}</div>
                        <div className="text-sm text-gray-500">
                          {pr.weight_kg}kg × {pr.reps} reps
                        </div>
                      </div>
                      <div className="text-right">
                        <div className="font-medium text-blue-600">
                          {pr.pr_type === '1rm' ? `${pr.value.toFixed(1)}kg 1RM` : pr.value}
                        </div>
                        <div className="text-xs text-gray-400">{pr.date}</div>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </>
        ) : (
          <>
            {/* Weekly Check-in */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Weekly Nutrition Check-in</h2>

              {checkinError && (
                <div className="mb-4 p-3 bg-yellow-50 border border-yellow-200 rounded-lg text-yellow-800 text-sm">
                  {checkinError}
                </div>
              )}

              {checkin ? (
                <div className="space-y-4">
                  <div className="p-4 bg-blue-50 rounded-lg">
                    <p className="text-blue-800">{checkin.summary}</p>
                  </div>

                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <div className="text-sm text-gray-500">Adaptive TDEE</div>
                      <div className="text-2xl font-bold">{Math.round(checkin.adaptive_tdee_kcal)}</div>
                    </div>
                    <div>
                      <div className="text-sm text-gray-500">Formula TDEE</div>
                      <div className="text-2xl font-bold text-gray-400">
                        {Math.round(checkin.formula_tdee_kcal)}
                      </div>
                    </div>
                  </div>

                  <div className="border-t pt-4">
                    <div className="text-sm text-gray-500 mb-2">Recommended Targets</div>
                    <div className="grid grid-cols-4 gap-2 text-center">
                      <div>
                        <div className="text-xl font-bold">
                          {Math.round(checkin.recommended_calorie_target)}
                        </div>
                        <div className="text-xs text-gray-500">kcal</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-blue-600">
                          {Math.round(checkin.protein_target_g)}
                        </div>
                        <div className="text-xs text-gray-500">protein</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-amber-600">
                          {Math.round(checkin.carbs_target_g)}
                        </div>
                        <div className="text-xs text-gray-500">carbs</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-red-600">
                          {Math.round(checkin.fat_target_g)}
                        </div>
                        <div className="text-xs text-gray-500">fat</div>
                      </div>
                    </div>
                  </div>

                  {checkin.calorie_adjustment !== 0 && (
                    <div className={`p-3 rounded-lg ${
                      checkin.calorie_adjustment > 0 ? 'bg-green-50 text-green-800' : 'bg-red-50 text-red-800'
                    }`}>
                      Adjustment: {checkin.calorie_adjustment > 0 ? '+' : ''}{checkin.calorie_adjustment} kcal/day
                    </div>
                  )}

                  <div className="flex gap-3">
                    <button className="flex-1 py-3 bg-green-600 text-white rounded-lg font-medium">
                      Accept Targets
                    </button>
                    <button className="flex-1 py-3 bg-gray-100 text-gray-700 rounded-lg font-medium">
                      Skip This Week
                    </button>
                  </div>
                </div>
              ) : (
                <div className="text-center py-8">
                  <p className="text-gray-500 mb-4">
                    Get personalized calorie and macro recommendations based on your actual progress.
                  </p>
                  <button
                    onClick={generateCheckin}
                    disabled={checkinLoading}
                    className="px-6 py-3 bg-blue-600 text-white rounded-lg font-medium disabled:opacity-50"
                  >
                    {checkinLoading ? 'Calculating...' : 'Generate Check-in'}
                  </button>
                </div>
              )}
            </div>

            {/* Calorie Adherence */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Calorie Adherence</h2>
              <div className="flex items-center justify-center">
                <div className="relative w-32 h-32">
                  <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#e5e7eb"
                      strokeWidth="12"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#22c55e"
                      strokeWidth="12"
                      strokeDasharray={`${78 * 2.51} ${100 * 2.51}`}
                      strokeLinecap="round"
                    />
                  </svg>
                  <div className="absolute inset-0 flex items-center justify-center">
                    <span className="text-2xl font-bold">78%</span>
                  </div>
                </div>
              </div>
              <p className="text-center text-gray-600 mt-4">
                You hit your calorie target <strong>78%</strong> of days this month
              </p>
            </div>

            {/* Protein Hit Rate */}
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Protein Target</h2>
              <div className="flex items-center justify-center">
                <div className="relative w-32 h-32">
                  <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#e5e7eb"
                      strokeWidth="12"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#3b82f6"
                      strokeWidth="12"
                      strokeDasharray={`${85 * 2.51} ${100 * 2.51}`}
                      strokeLinecap="round"
                    />
                  </svg>
                  <div className="absolute inset-0 flex items-center justify-center">
                    <span className="text-2xl font-bold">85%</span>
                  </div>
                </div>
              </div>
              <p className="text-center text-gray-600 mt-4">
                You hit ≥1.6g/kg protein on <strong>22 of 26</strong> days this month
              </p>
            </div>
          </>
        )}
      </div>
    </div>
  )
}
