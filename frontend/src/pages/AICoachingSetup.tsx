import { useState } from 'react'
import { api } from '../services/api'

interface QuestionnaireData {
  experience_level: string
  years_training: number
  current_frequency: number
  primary_goal: string
  secondary_goal?: string
  goal_timeframe_weeks?: number
  available_days_per_week: number
  session_duration_minutes: number
  available_equipment: string[]
  injuries_or_limitations?: string
  exercises_to_avoid: string[]
  preferred_rep_range?: string
  preferred_intensity?: string
}

export function AICoachingSetup() {
  const [step, setStep] = useState(1)
  const [formData, setFormData] = useState<QuestionnaireData>({
    experience_level: 'beginner',
    years_training: 0,
    current_frequency: 0,
    primary_goal: 'hypertrophy',
    available_days_per_week: 3,
    session_duration_minutes: 60,
    available_equipment: [],
    exercises_to_avoid: [],
  })

  const equipmentOptions = [
    'Barbell',
    'Dumbbells',
    'Cables',
    'Machines',
    'Bodyweight',
    'Resistance Bands',
    'Kettlebells',
    'Pull-up Bar',
  ]

  const updateField = (field: keyof QuestionnaireData, value: any) => {
    setFormData((prev) => ({ ...prev, [field]: value }))
  }

  const toggleEquipment = (equipment: string) => {
    setFormData((prev) => {
      const current = prev.available_equipment
      if (current.includes(equipment)) {
        return { ...prev, available_equipment: current.filter((e) => e !== equipment) }
      } else {
        return { ...prev, available_equipment: [...current, equipment] }
      }
    })
  }

  const submitQuestionnaire = async () => {
    try {
      await api.post('/api/coaching/questionnaire', formData)
      const response = await api.post('/api/coaching/generate-program', {})
      alert('Program generated! Redirecting to dashboard...')
      window.location.href = '/'
    } catch (error) {
      console.error('Failed to save questionnaire:', error)
      alert('Failed to generate program')
    }
  }

  const renderStep = () => {
    switch (step) {
      case 1:
        return (
          <div className="space-y-6">
            <h2 className="text-2xl font-bold text-text-primary">Training Experience</h2>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Experience Level
              </label>
              <select
                value={formData.experience_level}
                onChange={(e) => updateField('experience_level', e.target.value)}
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
              >
                <option value="beginner">Beginner (0-1 years)</option>
                <option value="intermediate">Intermediate (1-3 years)</option>
                <option value="advanced">Advanced (3+ years)</option>
              </select>
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Years Training: {formData.years_training}
              </label>
              <input
                type="range"
                min="0"
                max="10"
                step="0.5"
                value={formData.years_training}
                onChange={(e) => updateField('years_training', parseFloat(e.target.value))}
                className="w-full"
              />
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Current Training Frequency (days/week): {formData.current_frequency}
              </label>
              <input
                type="range"
                min="0"
                max="7"
                value={formData.current_frequency}
                onChange={(e) => updateField('current_frequency', parseInt(e.target.value))}
                className="w-full"
              />
            </div>
          </div>
        )

      case 2:
        return (
          <div className="space-y-6">
            <h2 className="text-2xl font-bold text-text-primary">Goals</h2>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Primary Goal
              </label>
              <select
                value={formData.primary_goal}
                onChange={(e) => updateField('primary_goal', e.target.value)}
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
              >
                <option value="strength">Strength</option>
                <option value="hypertrophy">Hypertrophy (Muscle Growth)</option>
                <option value="powerlifting">Powerlifting</option>
                <option value="bodybuilding">Bodybuilding</option>
                <option value="mixed">Mixed (Strength + Size)</option>
              </select>
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Goal Timeframe (weeks)
              </label>
              <input
                type="number"
                min="4"
                max="52"
                value={formData.goal_timeframe_weeks || ''}
                onChange={(e) => updateField('goal_timeframe_weeks', parseInt(e.target.value))}
                placeholder="e.g., 12"
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
              />
              <p className="text-xs text-text-secondary mt-1">
                How long do you want to follow this program?
              </p>
            </div>
          </div>
        )

      case 3:
        return (
          <div className="space-y-6">
            <h2 className="text-2xl font-bold text-text-primary">Schedule & Equipment</h2>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Available Days Per Week: {formData.available_days_per_week}
              </label>
              <input
                type="range"
                min="1"
                max="7"
                value={formData.available_days_per_week}
                onChange={(e) => updateField('available_days_per_week', parseInt(e.target.value))}
                className="w-full"
              />
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Session Duration (minutes): {formData.session_duration_minutes}
              </label>
              <input
                type="range"
                min="20"
                max="180"
                step="10"
                value={formData.session_duration_minutes}
                onChange={(e) => updateField('session_duration_minutes', parseInt(e.target.value))}
                className="w-full"
              />
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-3">
                Available Equipment (select all that apply)
              </label>
              <div className="grid grid-cols-2 gap-3">
                {equipmentOptions.map((equipment) => (
                  <button
                    key={equipment}
                    onClick={() => toggleEquipment(equipment)}
                    className={`px-4 py-2 rounded-lg border transition-colors ${
                      formData.available_equipment.includes(equipment)
                        ? 'bg-primary text-white border-primary'
                        : 'bg-surface text-text-primary border-border hover:border-primary'
                    }`}
                  >
                    {equipment}
                  </button>
                ))}
              </div>
            </div>
          </div>
        )

      case 4:
        return (
          <div className="space-y-6">
            <h2 className="text-2xl font-bold text-text-primary">
              Injuries & Preferences (Optional)
            </h2>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Injuries or Limitations
              </label>
              <textarea
                value={formData.injuries_or_limitations || ''}
                onChange={(e) => updateField('injuries_or_limitations', e.target.value)}
                placeholder="e.g., Lower back pain, shoulder mobility issues..."
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary h-24"
              />
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Preferred Rep Range
              </label>
              <select
                value={formData.preferred_rep_range || ''}
                onChange={(e) => updateField('preferred_rep_range', e.target.value)}
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
              >
                <option value="">No preference</option>
                <option value="low">Low (1-5 reps) - Strength focus</option>
                <option value="medium">Medium (6-12 reps) - Balanced</option>
                <option value="high">High (12-20 reps) - Endurance/pump</option>
              </select>
            </div>

            <div>
              <label className="block text-sm font-medium text-text-secondary mb-2">
                Preferred Intensity
              </label>
              <select
                value={formData.preferred_intensity || ''}
                onChange={(e) => updateField('preferred_intensity', e.target.value)}
                className="w-full bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
              >
                <option value="">No preference</option>
                <option value="low">Low - Leave reps in reserve</option>
                <option value="medium">Medium - Train close to failure</option>
                <option value="high">High - Frequent failure sets</option>
              </select>
            </div>
          </div>
        )

      default:
        return null
    }
  }

  return (
    <div className="min-h-screen bg-background">
      <div className="max-w-3xl mx-auto px-4 py-8">
        {/* Header */}
        <div className="mb-8">
          <h1 className="text-4xl font-bold text-text-primary mb-2">AI Coaching Setup</h1>
          <p className="text-text-secondary">
            Answer a few questions and we'll create the perfect program for you
          </p>
        </div>

        {/* Progress Bar */}
        <div className="mb-8">
          <div className="flex justify-between mb-2">
            <span className="text-sm text-text-secondary">Step {step} of 4</span>
            <span className="text-sm text-text-secondary">{Math.round((step / 4) * 100)}%</span>
          </div>
          <div className="w-full bg-surface h-2 rounded-full">
            <div
              className="bg-primary h-2 rounded-full transition-all duration-300"
              style={{ width: `${(step / 4) * 100}%` }}
            />
          </div>
        </div>

        {/* Form Content */}
        <div className="bg-surface border border-border rounded-lg p-6 mb-6">
          {renderStep()}
        </div>

        {/* Navigation Buttons */}
        <div className="flex justify-between">
          <button
            onClick={() => setStep(Math.max(1, step - 1))}
            disabled={step === 1}
            className="px-6 py-2 bg-surface border border-border text-text-primary rounded-lg disabled:opacity-50 disabled:cursor-not-allowed hover:bg-surface-light transition-colors"
          >
            Back
          </button>

          {step < 4 ? (
            <button
              onClick={() => setStep(step + 1)}
              className="px-6 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
            >
              Next
            </button>
          ) : (
            <button
              onClick={submitQuestionnaire}
              className="px-6 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
            >
              Generate My Program
            </button>
          )}
        </div>
      </div>
    </div>
  )
}
