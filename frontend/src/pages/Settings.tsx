import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { useQuery, useQueryClient } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { UserProfile } from '@/types'

interface SettingsProps {
  onLogout: () => void
}

function calculateTDEE(profile: UserProfile): number {
  const age = Math.floor(
    (Date.now() - new Date(profile.date_of_birth).getTime()) / (365.25 * 24 * 60 * 60 * 1000)
  )
  let bmr: number
  const baseBmr = 10 * profile.weight_kg + 6.25 * profile.height_cm - 5 * age - 161
  if (profile.sex === 'male') {
    bmr = baseBmr + 166 // -161 + 166 = +5 (Mifflin-St Jeor male offset)
  } else if (profile.sex === 'other') {
    bmr = baseBmr + 83 // midpoint between male and female
  } else {
    bmr = baseBmr
  }
  const multipliers: Record<string, number> = {
    sedentary: 1.2,
    lightly_active: 1.375,
    moderately_active: 1.55,
    very_active: 1.725,
    extremely_active: 1.9,
  }
  return Math.round(bmr * (multipliers[profile.activity_level] || 1.55))
}

export default function Settings({ onLogout }: SettingsProps) {
  const navigate = useNavigate()
  const queryClient = useQueryClient()
  const [saving, setSaving] = useState(false)
  const [saved, setSaved] = useState(false)
  const [form, setForm] = useState<Partial<UserProfile>>({})

  const { data, isLoading, error } = useQuery({
    queryKey: ['profile'],
    queryFn: () => api.getProfile(),
  })

  const profile = (data as { profile?: UserProfile } | undefined)?.profile ?? data as UserProfile | undefined

  useEffect(() => {
    if (profile) {
      setForm({
        date_of_birth: profile.date_of_birth,
        sex: profile.sex,
        height_cm: profile.height_cm,
        weight_kg: profile.weight_kg,
        activity_level: profile.activity_level,
        fitness_goal: profile.fitness_goal,
        unit_preference: profile.unit_preference,
      })
    }
  }, [profile])

  const handleSave = async () => {
    setSaving(true)
    setSaved(false)
    try {
      await api.updateProfile(form)
      queryClient.invalidateQueries({ queryKey: ['profile'] })
      queryClient.invalidateQueries({ queryKey: ['dashboard'] })
      setSaved(true)
      setTimeout(() => setSaved(false), 3000)
    } catch {
      alert('Failed to save profile')
    } finally {
      setSaving(false)
    }
  }

  const handleLogout = () => {
    onLogout()
    navigate('/login', { replace: true })
  }

  const tdee = profile ? calculateTDEE({ ...profile, ...form } as UserProfile) : 0

  if (isLoading) {
    return (
      <div className="min-h-screen bg-background flex items-center justify-center">
        <div className="text-text-muted">Loading profile...</div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="min-h-screen bg-background flex items-center justify-center">
        <div className="card max-w-sm text-center">
          <div className="text-danger mb-2">Failed to load profile</div>
          <button onClick={() => window.location.reload()} className="text-primary text-sm">Retry</button>
        </div>
      </div>
    )
  }

  return (
    <div className="min-h-screen bg-background pb-20">
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-2xl mx-auto px-4 py-4">
          <h1 className="text-2xl font-bold">Settings</h1>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-4 space-y-6">
        {/* Success toast */}
        {saved && (
          <div className="bg-success/10 border border-success rounded-lg p-3 text-center text-success text-sm">
            Profile saved successfully!
          </div>
        )}

        {/* Body Stats */}
        <section className="card">
          <h3 className="text-primary mb-4">Body Stats</h3>
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label htmlFor="settings-height" className="label">Height (cm)</label>
              <input
                id="settings-height"
                type="number"
                value={form.height_cm || ''}
                onChange={(e) => setForm({ ...form, height_cm: Number(e.target.value) })}
                className="input text-center font-mono"
              />
            </div>
            <div>
              <label htmlFor="settings-weight" className="label">Weight (kg)</label>
              <input
                id="settings-weight"
                type="number"
                value={form.weight_kg || ''}
                onChange={(e) => setForm({ ...form, weight_kg: Number(e.target.value) })}
                className="input text-center font-mono"
              />
            </div>
            <div>
              <label htmlFor="settings-dob" className="label">Date of Birth</label>
              <input
                id="settings-dob"
                type="date"
                value={form.date_of_birth || ''}
                onChange={(e) => setForm({ ...form, date_of_birth: e.target.value })}
                className="input"
              />
            </div>
            <div>
              <label htmlFor="settings-sex" className="label">Sex</label>
              <select
                id="settings-sex"
                value={form.sex || ''}
                onChange={(e) => setForm({ ...form, sex: e.target.value as UserProfile['sex'] })}
                className="input"
              >
                <option value="male">Male</option>
                <option value="female">Female</option>
                <option value="other">Other</option>
              </select>
            </div>
          </div>
        </section>

        {/* Goals */}
        <section className="card">
          <h3 className="text-primary mb-4">Goals</h3>
          <div className="space-y-4">
            <div>
              <label htmlFor="settings-activity" className="label">Activity Level</label>
              <select
                id="settings-activity"
                value={form.activity_level || ''}
                onChange={(e) => setForm({ ...form, activity_level: e.target.value as UserProfile['activity_level'] })}
                className="input"
              >
                <option value="sedentary">Sedentary</option>
                <option value="lightly_active">Lightly Active</option>
                <option value="moderately_active">Moderately Active</option>
                <option value="very_active">Very Active</option>
                <option value="extremely_active">Extremely Active</option>
              </select>
            </div>
            <div>
              <label htmlFor="settings-goal" className="label">Fitness Goal</label>
              <select
                id="settings-goal"
                value={form.fitness_goal || ''}
                onChange={(e) => setForm({ ...form, fitness_goal: e.target.value as UserProfile['fitness_goal'] })}
                className="input"
              >
                <option value="lose_fat">Lose Fat</option>
                <option value="maintain">Maintain</option>
                <option value="build_muscle">Build Muscle</option>
              </select>
            </div>
            <div>
              <label className="label">Units</label>
              <div className="flex gap-2">
                {(['metric', 'imperial'] as const).map((unit) => (
                  <button
                    key={unit}
                    type="button"
                    onClick={() => setForm({ ...form, unit_preference: unit })}
                    className={`tap-target flex-1 py-3 rounded-lg font-medium capitalize transition-all ${
                      form.unit_preference === unit
                        ? 'bg-primary text-white'
                        : 'bg-surface-elevated text-text-secondary'
                    }`}
                  >
                    {unit}
                  </button>
                ))}
              </div>
            </div>
          </div>
        </section>

        {/* Calculated Targets */}
        <section className="card">
          <h3 className="text-primary mb-4">Daily Targets</h3>
          <div className="text-center mb-3">
            <div className="text-xs text-text-muted">Estimated TDEE</div>
            <div className="text-2xl font-bold font-mono">{tdee} kcal</div>
          </div>
          {profile?.target_calories && (
            <div className="grid grid-cols-4 gap-3 text-center bg-surface-elevated rounded-lg p-3">
              <div>
                <div className="text-xs text-text-muted">Calories</div>
                <div className="font-bold font-mono text-primary">{profile.target_calories}</div>
              </div>
              <div>
                <div className="text-xs text-text-muted">Protein</div>
                <div className="font-bold font-mono text-success">{profile.target_protein_g}g</div>
              </div>
              <div>
                <div className="text-xs text-text-muted">Carbs</div>
                <div className="font-bold font-mono text-warning">{profile.target_carbs_g}g</div>
              </div>
              <div>
                <div className="text-xs text-text-muted">Fat</div>
                <div className="font-bold font-mono text-danger">{profile.target_fat_g}g</div>
              </div>
            </div>
          )}
          <div className="text-xs text-text-muted text-center mt-2">
            Targets are recalculated when you save changes
          </div>
        </section>

        {/* Save */}
        <button type="button" onClick={handleSave} disabled={saving} className="btn-primary w-full">
          {saving ? 'Saving...' : 'Save Changes'}
        </button>

        {/* Account */}
        <section className="card">
          <h3 className="text-danger mb-4">Account</h3>
          <button type="button" onClick={handleLogout} className="btn-secondary w-full mb-3">
            Log Out
          </button>
        </section>
      </div>
    </div>
  )
}
