import { useState, useEffect } from 'react'
import { api } from '../services/api'

interface Program {
  id: string
  name: string
  description: string
  goal: string
  experience_level: string
  duration_weeks: number
  workouts_per_week: number
  is_official: boolean
}

export function ProgramBrowser() {
  const [programs, setPrograms] = useState<Program[]>([])
  const [loading, setLoading] = useState(true)
  const [selectedGoal, setSelectedGoal] = useState<string>('all')
  const [selectedLevel, setSelectedLevel] = useState<string>('all')

  useEffect(() => {
    fetchPrograms()
  }, [selectedGoal, selectedLevel])

  const fetchPrograms = async () => {
    try {
      setLoading(true)
      const params = new URLSearchParams()
      if (selectedGoal !== 'all') params.append('goal', selectedGoal)
      if (selectedLevel !== 'all') params.append('experience_level', selectedLevel)

      const response = await api.get(`/api/programs?${params.toString()}`)
      setPrograms(response.programs)
    } catch (error) {
      console.error('Failed to fetch programs:', error)
    } finally {
      setLoading(false)
    }
  }

  const startProgram = async (programId: string) => {
    try {
      await api.post('/api/coaching/start-program', { program_id: programId })
      alert('Program started! Check your dashboard to begin.')
      window.location.href = '/'
    } catch (error) {
      console.error('Failed to start program:', error)
      alert('Failed to start program')
    }
  }

  const goalOptions = [
    { value: 'all', label: 'All Goals' },
    { value: 'strength', label: 'Strength' },
    { value: 'hypertrophy', label: 'Hypertrophy' },
    { value: 'mixed', label: 'Mixed' },
    { value: 'powerlifting', label: 'Powerlifting' },
    { value: 'bodybuilding', label: 'Bodybuilding' },
  ]

  const levelOptions = [
    { value: 'all', label: 'All Levels' },
    { value: 'beginner', label: 'Beginner' },
    { value: 'intermediate', label: 'Intermediate' },
    { value: 'advanced', label: 'Advanced' },
  ]

  return (
    <div className="min-h-screen bg-background">
      <div className="max-w-7xl mx-auto px-4 py-8">
        {/* Header */}
        <div className="mb-8">
          <h1 className="text-4xl font-bold text-text-primary mb-2">Browse Programs</h1>
          <p className="text-text-secondary">
            Choose from our curated programs or create your own
          </p>
        </div>

        {/* Filters */}
        <div className="flex flex-wrap gap-4 mb-8">
          <div>
            <label className="block text-sm font-medium text-text-secondary mb-2">
              Goal
            </label>
            <select
              value={selectedGoal}
              onChange={(e) => setSelectedGoal(e.target.value)}
              className="bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
            >
              {goalOptions.map((option) => (
                <option key={option.value} value={option.value}>
                  {option.label}
                </option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-text-secondary mb-2">
              Experience Level
            </label>
            <select
              value={selectedLevel}
              onChange={(e) => setSelectedLevel(e.target.value)}
              className="bg-surface border border-border rounded-lg px-4 py-2 text-text-primary"
            >
              {levelOptions.map((option) => (
                <option key={option.value} value={option.value}>
                  {option.label}
                </option>
              ))}
            </select>
          </div>
        </div>

        {/* Programs Grid */}
        {loading ? (
          <div className="text-center py-12">
            <div className="text-2xl text-text-secondary">Loading programs...</div>
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {programs.map((program) => (
              <div
                key={program.id}
                className="bg-surface border border-border rounded-lg p-6 hover:border-primary transition-colors"
              >
                {program.is_official && (
                  <div className="inline-block bg-primary text-white text-xs font-bold px-2 py-1 rounded mb-3">
                    OFFICIAL
                  </div>
                )}
                <h3 className="text-xl font-bold text-text-primary mb-2">
                  {program.name}
                </h3>
                <p className="text-text-secondary mb-4 text-sm">
                  {program.description}
                </p>

                <div className="space-y-2 mb-4">
                  <div className="flex justify-between text-sm">
                    <span className="text-text-secondary">Goal:</span>
                    <span className="text-text-primary font-medium capitalize">
                      {program.goal}
                    </span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-text-secondary">Level:</span>
                    <span className="text-text-primary font-medium capitalize">
                      {program.experience_level}
                    </span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-text-secondary">Duration:</span>
                    <span className="text-text-primary font-medium">
                      {program.duration_weeks} weeks
                    </span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-text-secondary">Frequency:</span>
                    <span className="text-text-primary font-medium">
                      {program.workouts_per_week}x/week
                    </span>
                  </div>
                </div>

                <button
                  onClick={() => startProgram(program.id)}
                  className="w-full bg-primary hover:bg-primary-dark text-white font-medium py-2 px-4 rounded-lg transition-colors"
                >
                  Start Program
                </button>
              </div>
            ))}
          </div>
        )}

        {!loading && programs.length === 0 && (
          <div className="text-center py-12">
            <p className="text-xl text-text-secondary">
              No programs found matching your criteria
            </p>
          </div>
        )}
      </div>
    </div>
  )
}
