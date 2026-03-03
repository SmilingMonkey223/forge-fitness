import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '../services/api'

export function WeeklyCheckIn() {
  const navigate = useNavigate()
  const [hungerRating, setHungerRating] = useState(3)
  const [energyRating, setEnergyRating] = useState(3)
  const [recoveryRating, setRecoveryRating] = useState(3)
  const [notes, setNotes] = useState('')
  const [isLoading, setIsLoading] = useState(false)
  const [successMessage, setSuccessMessage] = useState('')
  const [errorMessage, setErrorMessage] = useState('')

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setIsLoading(true)
    setSuccessMessage('')
    setErrorMessage('')

    try {
      await api.submitWeeklyCheckin({
        hunger_rating: hungerRating,
        energy_rating: energyRating,
        recovery_rating: recoveryRating,
        notes: notes || undefined,
      })

      setSuccessMessage('Check-in recorded successfully!')

      // Redirect to dashboard after 2 seconds
      setTimeout(() => {
        navigate('/')
      }, 2000)
    } catch {
      setErrorMessage('Failed to submit check-in. Please try again.')
    } finally {
      setIsLoading(false)
    }
  }

  const RatingSelector = ({
    label,
    value,
    onChange,
    lowLabel,
    highLabel,
  }: {
    label: string
    value: number
    onChange: (val: number) => void
    lowLabel: string
    highLabel: string
  }) => (
    <div className="mb-6">
      <label className="block text-text-primary font-semibold mb-2">{label}</label>
      <div className="flex items-center justify-between mb-2">
        <span className="text-sm text-text-secondary">{lowLabel}</span>
        <span className="text-sm text-text-secondary">{highLabel}</span>
      </div>
      <div className="flex gap-2">
        {[1, 2, 3, 4, 5].map((rating) => (
          <button
            key={rating}
            type="button"
            onClick={() => onChange(rating)}
            className={`flex-1 py-3 rounded-lg font-bold transition-colors ${
              value === rating
                ? 'bg-primary text-white'
                : 'bg-background border border-border text-text-secondary hover:border-primary'
            }`}
          >
            {rating}
          </button>
        ))}
      </div>
    </div>
  )

  return (
    <div className="min-h-screen bg-background">
      {/* Header */}
      <div className="bg-surface border-b border-border">
        <div className="max-w-7xl mx-auto px-4 py-4 flex justify-between items-center">
          <div className="flex items-center gap-4">
            <button
              onClick={() => navigate('/')}
              className="text-text-secondary hover:text-primary transition-colors"
            >
              ← Back
            </button>
            <h1 className="text-2xl font-bold text-text-primary">Weekly Check-In</h1>
          </div>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-8">
        {successMessage && (
          <div className="bg-green-500 bg-opacity-10 border border-green-500 rounded-lg p-4 mb-6">
            <p className="text-green-500">{successMessage}</p>
          </div>
        )}

        {errorMessage && (
          <div className="bg-red-500 bg-opacity-10 border border-red-500 rounded-lg p-4 mb-6">
            <p className="text-red-500">{errorMessage}</p>
          </div>
        )}

        <div className="bg-surface border border-border rounded-lg p-6 mb-6">
          <h2 className="text-xl font-bold text-text-primary mb-2">
            How was your week?
          </h2>
          <p className="text-text-secondary mb-6">
            This helps your AI coach adjust your program for optimal results
          </p>

          <form onSubmit={handleSubmit}>
            <RatingSelector
              label="Hunger Level"
              value={hungerRating}
              onChange={setHungerRating}
              lowLabel="Very hungry"
              highLabel="Very satisfied"
            />

            <RatingSelector
              label="Energy Level"
              value={energyRating}
              onChange={setEnergyRating}
              lowLabel="Exhausted"
              highLabel="Energetic"
            />

            <RatingSelector
              label="Recovery Quality"
              value={recoveryRating}
              onChange={setRecoveryRating}
              lowLabel="Poor recovery"
              highLabel="Great recovery"
            />

            {/* Notes */}
            <div className="mb-6">
              <label className="block text-text-primary font-semibold mb-2">
                Additional Notes (Optional)
              </label>
              <textarea
                value={notes}
                onChange={(e) => setNotes(e.target.value)}
                placeholder="Any injuries, life stressors, or observations from this week..."
                rows={4}
                className="w-full px-4 py-2 bg-background border border-border rounded-lg text-text-primary focus:outline-none focus:border-primary"
              />
            </div>

            {/* Submit Button */}
            <button
              type="submit"
              disabled={isLoading}
              className="w-full px-6 py-3 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors disabled:opacity-50"
            >
              {isLoading ? 'Submitting...' : 'Submit Check-In'}
            </button>
          </form>
        </div>

        <div className="bg-blue-500 bg-opacity-10 border border-blue-500 rounded-lg p-4">
          <h3 className="font-bold text-blue-500 mb-2">💡 Why Check In?</h3>
          <ul className="text-sm text-text-secondary space-y-1">
            <li>• Helps adjust training volume and intensity</li>
            <li>• Optimizes nutrition recommendations</li>
            <li>• Prevents overtraining and burnout</li>
            <li>• Maximizes long-term progress</li>
          </ul>
        </div>
      </div>
    </div>
  )
}
