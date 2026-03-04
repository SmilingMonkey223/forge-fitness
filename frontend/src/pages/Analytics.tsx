import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type {
  MuscleGroupVolume,
  PersonalRecord,
  ConsistencyPoint,
  Streaks,
  CheckInRecommendation,
  NutritionSummary,
  UserProfile
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
  const [nutritionSummaries, setNutritionSummaries] = useState<NutritionSummary[]>([])
  const [acceptingTargets, setAcceptingTargets] = useState(false)
  const [acceptSuccess, setAcceptSuccess] = useState(false)
  const [profile, setProfile] = useState<UserProfile | null>(null)

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
      } else {
        const now = new Date()
        const end = now.toISOString().split('T')[0]
        const thirtyDaysAgo = new Date(now.getTime() - 30 * 24 * 60 * 60 * 1000)
        const start = thirtyDaysAgo.toISOString().split('T')[0]
        const [summaries, profileRes] = await Promise.all([
          api.getNutritionSummaryRange(start, end),
          api.getProfile()
        ])
        setNutritionSummaries(summaries)
        setProfile(profileRes.profile)
      }
    } catch {
      // Silently handle errors - data will remain in its default state
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

  const acceptTargets = async () => {
    if (!checkin) return
    setAcceptingTargets(true)
    setAcceptSuccess(false)
    try {
      const result = await api.updateProfile({
        target_calories: Math.round(checkin.recommended_calorie_target),
        target_protein_g: Math.round(checkin.protein_target_g),
        target_carbs_g: Math.round(checkin.carbs_target_g),
        target_fat_g: Math.round(checkin.fat_target_g),
        tdee_calories: Math.round(checkin.adaptive_tdee_kcal),
      })
      setProfile(result.profile)
      setAcceptSuccess(true)
      setTimeout(() => setAcceptSuccess(false), 3000)
    } catch {
      // Silently handle errors
    } finally {
      setAcceptingTargets(false)
    }
  }

  const computeCalorieAdherence = (): { percent: number; daysHit: number; totalDays: number } => {
    const target = checkin?.recommended_calorie_target ?? profile?.target_calories
    if (!target || nutritionSummaries.length === 0) return { percent: 0, daysHit: 0, totalDays: 0 }
    const tolerance = target * 0.1
    const daysHit = nutritionSummaries.filter(
      s => s.total_calories >= target - tolerance && s.total_calories <= target + tolerance
    ).length
    const totalDays = nutritionSummaries.length
    const percent = totalDays > 0 ? Math.round((daysHit / totalDays) * 100) : 0
    return { percent, daysHit, totalDays }
  }

  const computeProteinHitRate = (): { percent: number; daysHit: number; totalDays: number } => {
    const target = checkin?.protein_target_g ?? profile?.target_protein_g
    if (!target || nutritionSummaries.length === 0) return { percent: 0, daysHit: 0, totalDays: 0 }
    const threshold = target * 0.9
    const daysHit = nutritionSummaries.filter(s => s.total_protein_g >= threshold).length
    const totalDays = nutritionSummaries.length
    const percent = totalDays > 0 ? Math.round((daysHit / totalDays) * 100) : 0
    return { percent, daysHit, totalDays }
  }

  const getMuscleColor = (index: number) => {
    const colors = [
      'bg-primary', 'bg-success', 'bg-purple-500', 'bg-warning',
      'bg-pink-500', 'bg-cyan-500', 'bg-celebration', 'bg-danger'
    ]
    return colors[index % colors.length]
  }

  const getConsistencyColor = (point: ConsistencyPoint) => {
    if (!point.workout_completed) return 'bg-surface-elevated'
    if (point.volume_kg > 5000) return 'bg-success'
    if (point.volume_kg > 2500) return 'bg-success/60'
    return 'bg-success/30'
  }

  const calAdherence = computeCalorieAdherence()
  const proteinRate = computeProteinHitRate()

  return (
    <div className="min-h-screen bg-background pb-20">
      {/* Header */}
      <div className="bg-surface shadow-card">
        <div className="max-w-lg mx-auto px-4 py-4 flex items-center justify-between">
          <button onClick={() => navigate(-1)} className="text-text-secondary">
            ← Back
          </button>
          <h1 className="text-lg font-semibold text-text-primary">Analytics</h1>
          <div className="w-10" />
        </div>

        {/* Tabs */}
        <div className="max-w-lg mx-auto px-4">
          <div className="flex border-b border-border">
            <button
              onClick={() => setActiveTab('training')}
              className={`flex-1 py-3 text-center font-medium border-b-2 transition-colors ${
                activeTab === 'training'
                  ? 'border-primary text-primary'
                  : 'border-transparent text-text-muted'
              }`}
            >
              Training
            </button>
            <button
              onClick={() => setActiveTab('nutrition')}
              className={`flex-1 py-3 text-center font-medium border-b-2 transition-colors ${
                activeTab === 'nutrition'
                  ? 'border-primary text-primary'
                  : 'border-transparent text-text-muted'
              }`}
            >
              Nutrition
            </button>
          </div>
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-6 space-y-6">
        {loading ? (
          <div className="flex items-center justify-center h-48 text-text-muted">
            Loading...
          </div>
        ) : activeTab === 'training' ? (
          <>
            {/* Streak Cards */}
            {streaks && (
              <div className="grid grid-cols-3 gap-3">
                <div className="bg-surface rounded-card shadow-card p-4 text-center">
                  <div className="text-3xl font-bold text-warning">
                    {streaks.workout.current}
                  </div>
                  <div className="text-xs text-text-secondary mt-1">Workout Streak</div>
                  <div className="text-xs text-text-muted">Best: {streaks.workout.best}</div>
                </div>
                <div className="bg-surface rounded-card shadow-card p-4 text-center">
                  <div className="text-3xl font-bold text-success">
                    {streaks.logging.current}
                  </div>
                  <div className="text-xs text-text-secondary mt-1">Logging Streak</div>
                  <div className="text-xs text-text-muted">Best: {streaks.logging.best}</div>
                </div>
                <div className="bg-surface rounded-card shadow-card p-4 text-center">
                  <div className="text-3xl font-bold text-primary">
                    {streaks.weight.current}
                  </div>
                  <div className="text-xs text-text-secondary mt-1">Weight Streak</div>
                  <div className="text-xs text-text-muted">Best: {streaks.weight.best}</div>
                </div>
              </div>
            )}

            {/* Consistency Heatmap */}
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Training Consistency</h2>
              <div className="grid grid-cols-7 gap-1">
                {['S', 'M', 'T', 'W', 'T', 'F', 'S'].map((day, i) => (
                  <div key={i} className="text-center text-xs text-text-muted mb-1">
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
              <div className="mt-4 flex justify-end gap-2 text-xs text-text-secondary">
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-surface-elevated" /> Rest
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-success/30" /> Light
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-success/60" /> Medium
                </div>
                <div className="flex items-center gap-1">
                  <div className="w-3 h-3 rounded-sm bg-success" /> Heavy
                </div>
              </div>
            </div>

            {/* Muscle Distribution */}
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Muscle Distribution (4 weeks)</h2>
              {muscleDistribution.length === 0 ? (
                <p className="text-text-muted text-center py-8">
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
                      <div className="w-24 h-24 bg-surface rounded-full flex items-center justify-center">
                        <span className="text-lg font-bold text-text-secondary">
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
                        <span className="text-sm text-text-secondary flex-1 capitalize">
                          {group.muscle_group.replace('_', ' ')}
                        </span>
                        <span className="text-sm font-medium text-text-primary">{group.total_sets}</span>
                      </div>
                    ))}
                  </div>
                </>
              )}
            </div>

            {/* Recent PRs */}
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Recent PRs</h2>
              {prs.length === 0 ? (
                <p className="text-text-muted text-center py-8">
                  No personal records yet. Keep training!
                </p>
              ) : (
                <div className="space-y-3">
                  {prs.map(pr => (
                    <div key={pr.id} className="flex items-center gap-3 py-2 border-b border-border last:border-b-0">
                      <div className="text-2xl">🏆</div>
                      <div className="flex-1">
                        <div className="font-medium text-text-primary">{pr.exercise_name}</div>
                        <div className="text-sm text-text-secondary">
                          {pr.weight_kg}kg × {pr.reps} reps
                        </div>
                      </div>
                      <div className="text-right">
                        <div className="font-medium text-primary">
                          {pr.pr_type === '1rm' ? `${pr.value.toFixed(1)}kg 1RM` : pr.value}
                        </div>
                        <div className="text-xs text-text-muted">{pr.date}</div>
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
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Weekly Nutrition Check-in</h2>

              {checkinError && (
                <div className="mb-4 p-3 bg-warning/10 border border-warning/30 rounded-lg text-warning text-sm">
                  {checkinError}
                </div>
              )}

              {checkin ? (
                <div className="space-y-4">
                  <div className="p-4 bg-primary/10 rounded-lg">
                    <p className="text-primary">{checkin.summary}</p>
                  </div>

                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <div className="text-sm text-text-secondary">Adaptive TDEE</div>
                      <div className="text-2xl font-bold text-text-primary">{Math.round(checkin.adaptive_tdee_kcal)}</div>
                    </div>
                    <div>
                      <div className="text-sm text-text-secondary">Formula TDEE</div>
                      <div className="text-2xl font-bold text-text-muted">
                        {Math.round(checkin.formula_tdee_kcal)}
                      </div>
                    </div>
                  </div>

                  <div className="border-t border-border pt-4">
                    <div className="text-sm text-text-secondary mb-2">Recommended Targets</div>
                    <div className="grid grid-cols-4 gap-2 text-center">
                      <div>
                        <div className="text-xl font-bold text-text-primary">
                          {Math.round(checkin.recommended_calorie_target)}
                        </div>
                        <div className="text-xs text-text-secondary">kcal</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-primary">
                          {Math.round(checkin.protein_target_g)}
                        </div>
                        <div className="text-xs text-text-secondary">protein</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-warning">
                          {Math.round(checkin.carbs_target_g)}
                        </div>
                        <div className="text-xs text-text-secondary">carbs</div>
                      </div>
                      <div>
                        <div className="text-xl font-bold text-danger">
                          {Math.round(checkin.fat_target_g)}
                        </div>
                        <div className="text-xs text-text-secondary">fat</div>
                      </div>
                    </div>
                  </div>

                  {checkin.calorie_adjustment !== 0 && (
                    <div className={`p-3 rounded-lg ${
                      checkin.calorie_adjustment > 0 ? 'bg-success/10 text-success' : 'bg-danger/10 text-danger'
                    }`}>
                      Adjustment: {checkin.calorie_adjustment > 0 ? '+' : ''}{checkin.calorie_adjustment} kcal/day
                    </div>
                  )}

                  {acceptSuccess && (
                    <div className="p-3 bg-success/10 border border-success/30 rounded-lg text-success text-sm">
                      Targets updated successfully!
                    </div>
                  )}

                  <div className="flex gap-3">
                    <button
                      onClick={acceptTargets}
                      disabled={acceptingTargets || acceptSuccess}
                      className="flex-1 py-3 bg-success text-background rounded-lg font-medium disabled:opacity-50"
                    >
                      {acceptingTargets ? 'Saving...' : acceptSuccess ? 'Saved' : 'Accept Targets'}
                    </button>
                    <button className="flex-1 py-3 bg-surface-elevated text-text-secondary rounded-lg font-medium">
                      Skip This Week
                    </button>
                  </div>
                </div>
              ) : (
                <div className="text-center py-8">
                  <p className="text-text-secondary mb-4">
                    Get personalized calorie and macro recommendations based on your actual progress.
                  </p>
                  <button
                    onClick={generateCheckin}
                    disabled={checkinLoading}
                    className="px-6 py-3 bg-primary text-text-primary rounded-lg font-medium disabled:opacity-50"
                  >
                    {checkinLoading ? 'Calculating...' : 'Generate Check-in'}
                  </button>
                </div>
              )}
            </div>

            {/* Calorie Adherence */}
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Calorie Adherence</h2>
              <div className="flex items-center justify-center">
                <div className="relative w-32 h-32">
                  <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#2A2A3E"
                      strokeWidth="12"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#00D68F"
                      strokeWidth="12"
                      strokeDasharray={`${calAdherence.percent * 2.51} ${100 * 2.51}`}
                      strokeLinecap="round"
                    />
                  </svg>
                  <div className="absolute inset-0 flex items-center justify-center">
                    <span className="text-2xl font-bold text-text-primary">{calAdherence.percent}%</span>
                  </div>
                </div>
              </div>
              <p className="text-center text-text-secondary mt-4">
                {nutritionSummaries.length > 0
                  ? <>You hit your calorie target <strong>{calAdherence.percent}%</strong> of days this month ({calAdherence.daysHit} of {calAdherence.totalDays})</>
                  : 'Log your nutrition to see calorie adherence data'}
              </p>
            </div>

            {/* Protein Hit Rate */}
            <div className="bg-surface rounded-card shadow-card p-6">
              <h2 className="text-lg font-medium text-text-primary mb-4">Protein Target</h2>
              <div className="flex items-center justify-center">
                <div className="relative w-32 h-32">
                  <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#2A2A3E"
                      strokeWidth="12"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      fill="none"
                      stroke="#6C5CE7"
                      strokeWidth="12"
                      strokeDasharray={`${proteinRate.percent * 2.51} ${100 * 2.51}`}
                      strokeLinecap="round"
                    />
                  </svg>
                  <div className="absolute inset-0 flex items-center justify-center">
                    <span className="text-2xl font-bold text-text-primary">{proteinRate.percent}%</span>
                  </div>
                </div>
              </div>
              <p className="text-center text-text-secondary mt-4">
                {nutritionSummaries.length > 0
                  ? <>You hit your protein target on <strong>{proteinRate.daysHit} of {proteinRate.totalDays}</strong> days this month</>
                  : 'Log your nutrition to see protein adherence data'}
              </p>
            </div>
          </>
        )}
      </div>
    </div>
  )
}
