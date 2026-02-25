import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '@/services/api'

interface OnboardingProps {
  onComplete: () => void
}

interface OnboardingData {
  sex: 'male' | 'female' | ''
  date_of_birth: string
  height_cm: number | ''
  weight_kg: number | ''
  activity_level: string
  fitness_goal: string
}

const ACTIVITY_LEVELS = [
  { value: 'sedentary', label: 'Sedentary', description: 'Little or no exercise, desk job' },
  { value: 'lightly_active', label: 'Lightly Active', description: 'Light exercise 1-3 days/week' },
  { value: 'moderately_active', label: 'Moderately Active', description: 'Moderate exercise 3-5 days/week' },
  { value: 'very_active', label: 'Very Active', description: 'Hard exercise 6-7 days/week' },
  { value: 'extremely_active', label: 'Extremely Active', description: 'Very hard exercise, physical job' },
]

const FITNESS_GOALS = [
  { value: 'lose_fat', label: 'Lose Fat', description: 'Calorie deficit to reduce body fat', icon: '\u2193' },
  { value: 'maintain', label: 'Maintain', description: 'Stay at your current weight', icon: '\u2194' },
  { value: 'build_muscle', label: 'Build Muscle', description: 'Calorie surplus to gain muscle', icon: '\u2191' },
]

const STEP_COUNT = 5

const slideVariants = {
  enter: (direction: number) => ({
    x: direction > 0 ? 300 : -300,
    opacity: 0,
  }),
  center: {
    x: 0,
    opacity: 1,
  },
  exit: (direction: number) => ({
    x: direction < 0 ? 300 : -300,
    opacity: 0,
  }),
}

export default function Onboarding({ onComplete }: OnboardingProps) {
  const navigate = useNavigate()
  const [step, setStep] = useState(0)
  const [direction, setDirection] = useState(0)
  const [error, setError] = useState('')
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [results, setResults] = useState<{
    tdee_calories: number
    target_calories: number
    target_protein_g: number
    target_carbs_g: number
    target_fat_g: number
  } | null>(null)

  const [data, setData] = useState<OnboardingData>({
    sex: '',
    date_of_birth: '',
    height_cm: '',
    weight_kg: '',
    activity_level: '',
    fitness_goal: '',
  })

  const goNext = () => {
    setDirection(1)
    setError('')
    setStep((s) => Math.min(s + 1, STEP_COUNT - 1))
  }

  const goBack = () => {
    setDirection(-1)
    setError('')
    setStep((s) => Math.max(s - 1, 0))
  }

  const canProceed = (): boolean => {
    switch (step) {
      case 0:
        return true // Welcome step
      case 1:
        return data.sex !== '' && data.date_of_birth !== '' && data.height_cm !== '' && data.weight_kg !== ''
      case 2:
        return data.activity_level !== ''
      case 3:
        return data.fitness_goal !== ''
      case 4:
        return true // Results step
      default:
        return false
    }
  }

  const handleSubmitOnboarding = async () => {
    if (step === 3) {
      // Submit to get calculated results before showing step 5
      setIsSubmitting(true)
      setError('')
      try {
        const response = await api.completeOnboarding({
          date_of_birth: data.date_of_birth,
          sex: data.sex,
          height_cm: Number(data.height_cm),
          weight_kg: Number(data.weight_kg),
          activity_level: data.activity_level,
          fitness_goal: data.fitness_goal,
        })

        setResults({
          tdee_calories: response.profile.tdee_calories || 0,
          target_calories: response.profile.target_calories || 0,
          target_protein_g: response.profile.target_protein_g || 0,
          target_carbs_g: response.profile.target_carbs_g || 0,
          target_fat_g: response.profile.target_fat_g || 0,
        })

        goNext()
      } catch (err: any) {
        const code = err.message
        if (code === 'PROFILE_ALREADY_EXISTS') {
          setError('Profile already exists. Redirecting to dashboard...')
          setTimeout(() => {
            onComplete()
            navigate('/')
          }, 1500)
        } else if (code === 'INVALID_DATE_OF_BIRTH' || code === 'INVALID_AGE') {
          setError('Please enter a valid date of birth (you must be at least 13 years old).')
        } else if (code === 'INVALID_HEIGHT') {
          setError('Height must be between 100 and 250 cm.')
        } else if (code === 'INVALID_WEIGHT') {
          setError('Weight must be between 30 and 300 kg.')
        } else {
          setError('Something went wrong. Please try again.')
        }
      } finally {
        setIsSubmitting(false)
      }
    }
  }

  const handleFinish = () => {
    onComplete()
    navigate('/')
  }

  const ProgressBar = () => (
    <div className="flex gap-2 mb-8">
      {Array.from({ length: STEP_COUNT }).map((_, i) => (
        <div
          key={i}
          className={`h-1 flex-1 rounded-full transition-colors duration-300 ${
            i <= step ? 'bg-primary' : 'bg-surface-elevated'
          }`}
        />
      ))}
    </div>
  )

  const WelcomeStep = () => (
    <div className="text-center py-8">
      <h1 className="text-3xl font-bold mb-4">Welcome to FORGE</h1>
      <p className="text-text-secondary text-lg mb-2">
        Your journey to a stronger you starts here.
      </p>
      <p className="text-text-muted mb-8">
        Let's set up your profile so we can personalize your experience
        with accurate calorie and macro targets.
      </p>
      <div className="w-24 h-24 mx-auto mb-8 rounded-full bg-primary bg-opacity-20 flex items-center justify-center">
        <span className="text-4xl text-primary font-bold">F</span>
      </div>
      <p className="text-text-muted text-sm">
        This takes less than a minute.
      </p>
    </div>
  )

  const BasicInfoStep = () => (
    <div>
      <h2 className="text-2xl font-bold mb-2">Basic Information</h2>
      <p className="text-text-secondary mb-6">Tell us about yourself so we can calculate your needs.</p>

      <div className="space-y-5">
        {/* Sex */}
        <div>
          <label className="label">Sex</label>
          <div className="grid grid-cols-2 gap-3">
            {(['male', 'female'] as const).map((option) => (
              <button
                key={option}
                type="button"
                onClick={() => setData({ ...data, sex: option })}
                className={`py-3 px-4 rounded-lg border text-center font-medium transition-all ${
                  data.sex === option
                    ? 'border-primary bg-primary bg-opacity-10 text-primary'
                    : 'border-text-muted bg-surface-elevated text-text-secondary hover:border-primary'
                }`}
              >
                {option === 'male' ? 'Male' : 'Female'}
              </button>
            ))}
          </div>
        </div>

        {/* Date of Birth */}
        <div>
          <label className="label">Date of Birth</label>
          <input
            type="date"
            className="input"
            value={data.date_of_birth}
            onChange={(e) => setData({ ...data, date_of_birth: e.target.value })}
            max={new Date(new Date().setFullYear(new Date().getFullYear() - 13)).toISOString().split('T')[0]}
          />
        </div>

        {/* Height */}
        <div>
          <label className="label">Height (cm)</label>
          <input
            type="number"
            className="input"
            placeholder="e.g. 175"
            value={data.height_cm}
            onChange={(e) => setData({ ...data, height_cm: e.target.value ? Number(e.target.value) : '' })}
            min={100}
            max={250}
            step={0.1}
          />
        </div>

        {/* Weight */}
        <div>
          <label className="label">Weight (kg)</label>
          <input
            type="number"
            className="input"
            placeholder="e.g. 80"
            value={data.weight_kg}
            onChange={(e) => setData({ ...data, weight_kg: e.target.value ? Number(e.target.value) : '' })}
            min={30}
            max={300}
            step={0.1}
          />
        </div>
      </div>
    </div>
  )

  const ActivityLevelStep = () => (
    <div>
      <h2 className="text-2xl font-bold mb-2">Activity Level</h2>
      <p className="text-text-secondary mb-6">How active are you on a typical week?</p>

      <div className="space-y-3">
        {ACTIVITY_LEVELS.map((level) => (
          <button
            key={level.value}
            type="button"
            onClick={() => setData({ ...data, activity_level: level.value })}
            className={`w-full text-left p-4 rounded-lg border transition-all ${
              data.activity_level === level.value
                ? 'border-primary bg-primary bg-opacity-10'
                : 'border-text-muted bg-surface-elevated hover:border-primary'
            }`}
          >
            <div className={`font-medium ${
              data.activity_level === level.value ? 'text-primary' : 'text-text-primary'
            }`}>
              {level.label}
            </div>
            <div className="text-sm text-text-muted mt-1">{level.description}</div>
          </button>
        ))}
      </div>
    </div>
  )

  const FitnessGoalStep = () => (
    <div>
      <h2 className="text-2xl font-bold mb-2">Fitness Goal</h2>
      <p className="text-text-secondary mb-6">What is your primary goal right now?</p>

      <div className="space-y-3">
        {FITNESS_GOALS.map((goal) => (
          <button
            key={goal.value}
            type="button"
            onClick={() => setData({ ...data, fitness_goal: goal.value })}
            className={`w-full text-left p-5 rounded-lg border transition-all ${
              data.fitness_goal === goal.value
                ? 'border-primary bg-primary bg-opacity-10'
                : 'border-text-muted bg-surface-elevated hover:border-primary'
            }`}
          >
            <div className="flex items-center gap-4">
              <span className={`text-2xl ${
                data.fitness_goal === goal.value ? 'text-primary' : 'text-text-muted'
              }`}>
                {goal.icon}
              </span>
              <div>
                <div className={`font-medium text-lg ${
                  data.fitness_goal === goal.value ? 'text-primary' : 'text-text-primary'
                }`}>
                  {goal.label}
                </div>
                <div className="text-sm text-text-muted mt-1">{goal.description}</div>
              </div>
            </div>
          </button>
        ))}
      </div>
    </div>
  )

  const ResultsStep = () => {
    if (!results) return null

    const macroData = [
      { label: 'Protein', value: results.target_protein_g, unit: 'g', color: '#00D68F' },
      { label: 'Carbs', value: results.target_carbs_g, unit: 'g', color: '#FFB800' },
      { label: 'Fat', value: results.target_fat_g, unit: 'g', color: '#FF5252' },
    ]

    return (
      <div className="text-center">
        <h2 className="text-2xl font-bold mb-2">Your Personalized Plan</h2>
        <p className="text-text-secondary mb-8">Based on your profile, here are your daily targets.</p>

        {/* TDEE */}
        <div className="card mb-6">
          <div className="text-text-muted text-sm mb-1">Total Daily Energy Expenditure</div>
          <div className="text-3xl font-bold font-mono text-primary">
            {Math.round(results.tdee_calories)}
          </div>
          <div className="text-text-muted text-sm">calories/day</div>
        </div>

        {/* Target Calories */}
        <div className="card mb-6">
          <div className="text-text-muted text-sm mb-1">Daily Calorie Target</div>
          <div className="text-4xl font-bold font-mono text-primary">
            {Math.round(results.target_calories)}
          </div>
          <div className="text-text-muted text-sm">calories/day</div>
        </div>

        {/* Macro Breakdown */}
        <div className="grid grid-cols-3 gap-4 mb-8">
          {macroData.map((macro) => (
            <div key={macro.label} className="card py-4 px-3">
              <div
                className="text-2xl font-bold font-mono mb-1"
                style={{ color: macro.color }}
              >
                {Math.round(macro.value)}
                <span className="text-sm text-text-muted">{macro.unit}</span>
              </div>
              <div className="text-sm text-text-secondary">{macro.label}</div>
            </div>
          ))}
        </div>

        <p className="text-text-muted text-sm">
          You can adjust these targets anytime from your profile settings.
        </p>
      </div>
    )
  }

  const renderStep = () => {
    switch (step) {
      case 0:
        return <WelcomeStep />
      case 1:
        return <BasicInfoStep />
      case 2:
        return <ActivityLevelStep />
      case 3:
        return <FitnessGoalStep />
      case 4:
        return <ResultsStep />
      default:
        return null
    }
  }

  return (
    <div className="min-h-screen flex items-center justify-center px-4 py-8">
      <div className="max-w-lg w-full">
        <ProgressBar />

        <div className="card min-h-[400px] flex flex-col">
          {/* Step content with animation */}
          <div className="flex-1 overflow-hidden">
            <AnimatePresence mode="wait" custom={direction}>
              <motion.div
                key={step}
                custom={direction}
                variants={slideVariants}
                initial="enter"
                animate="center"
                exit="exit"
                transition={{ duration: 0.3, ease: 'easeInOut' }}
              >
                {renderStep()}
              </motion.div>
            </AnimatePresence>
          </div>

          {/* Error display */}
          {error && (
            <div className="bg-danger bg-opacity-10 border border-danger text-danger px-4 py-3 rounded mt-4">
              {error}
            </div>
          )}

          {/* Navigation buttons */}
          <div className="flex justify-between items-center mt-6 pt-4 border-t border-text-muted border-opacity-20">
            {step > 0 && step < 4 ? (
              <button
                type="button"
                onClick={goBack}
                className="btn-secondary"
              >
                Back
              </button>
            ) : (
              <div />
            )}

            {step === 0 && (
              <button
                type="button"
                onClick={goNext}
                className="btn-primary ml-auto"
              >
                Get Started
              </button>
            )}

            {step >= 1 && step <= 2 && (
              <button
                type="button"
                onClick={goNext}
                disabled={!canProceed()}
                className={`btn-primary ${!canProceed() ? 'opacity-50 cursor-not-allowed' : ''}`}
              >
                Continue
              </button>
            )}

            {step === 3 && (
              <button
                type="button"
                onClick={handleSubmitOnboarding}
                disabled={!canProceed() || isSubmitting}
                className={`btn-primary ${(!canProceed() || isSubmitting) ? 'opacity-50 cursor-not-allowed' : ''}`}
              >
                {isSubmitting ? 'Calculating...' : 'Calculate My Plan'}
              </button>
            )}

            {step === 4 && (
              <button
                type="button"
                onClick={handleFinish}
                className="btn-primary ml-auto"
              >
                Go to Dashboard
              </button>
            )}
          </div>
        </div>
      </div>
    </div>
  )
}
