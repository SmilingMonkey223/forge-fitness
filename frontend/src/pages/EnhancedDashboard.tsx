import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '../services/api'
import { VolumeChart } from '../components/VolumeChart'
import { PRList } from '../components/PRList'
import { ExerciseProgressChart } from '../components/ExerciseProgressChart'
import { BodyMap } from '../components/BodyMap'

interface DashboardProps {
  onLogout: () => void
}

export function EnhancedDashboard({ onLogout }: DashboardProps) {
  const navigate = useNavigate()
  const [volumeData, setVolumeData] = useState([])
  const [prData, setPRData] = useState([])
  const [progressData, setProgressData] = useState([])
  const [coachingState, setCoachingState] = useState<any>(null)
  const [selectedExercise, setSelectedExercise] = useState('Bench Press')

  useEffect(() => {
    fetchDashboardData()
  }, [])

  const fetchDashboardData = async () => {
    try {
      // Fetch coaching state
      const state = await api.get('/api/coaching/state')
      setCoachingState(state)

      // Mock data for demonstration (replace with real API calls)
      setVolumeData([
        { muscle_group: 'chest', total_volume: 12500, exercise_count: 4, total_reps: 120 },
        { muscle_group: 'back', total_volume: 15000, exercise_count: 5, total_reps: 145 },
        { muscle_group: 'legs', total_volume: 22000, exercise_count: 6, total_reps: 180 },
        { muscle_group: 'shoulders', total_volume: 8500, exercise_count: 3, total_reps: 95 },
        { muscle_group: 'arms', total_volume: 6000, exercise_count: 4, total_reps: 110 },
        { muscle_group: 'core', total_volume: 3000, exercise_count: 2, total_reps: 150 },
      ])

      setPRData([
        {
          exercise_name: 'Bench Press',
          weight_lbs: 225,
          reps: 5,
          estimated_1rm: 253,
          achieved_at: new Date(Date.now() - 2 * 24 * 60 * 60 * 1000).toISOString(),
        },
        {
          exercise_name: 'Squat',
          weight_lbs: 315,
          reps: 3,
          estimated_1rm: 343,
          achieved_at: new Date(Date.now() - 5 * 24 * 60 * 60 * 1000).toISOString(),
        },
        {
          exercise_name: 'Deadlift',
          weight_lbs: 405,
          reps: 1,
          estimated_1rm: 405,
          achieved_at: new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toISOString(),
        },
      ])

      setProgressData([
        { date: '2025-01-01', estimated_1rm: 225, weight_lbs: 185, reps: 5 },
        { date: '2025-01-08', estimated_1rm: 230, weight_lbs: 190, reps: 5 },
        { date: '2025-01-15', estimated_1rm: 235, weight_lbs: 195, reps: 5 },
        { date: '2025-01-22', estimated_1rm: 242, weight_lbs: 200, reps: 5 },
        { date: '2025-01-29', estimated_1rm: 248, weight_lbs: 205, reps: 5 },
        { date: '2025-02-05', estimated_1rm: 253, weight_lbs: 225, reps: 5 },
      ])
    } catch (error) {
      console.error('Failed to fetch dashboard data:', error)
    }
  }

  const handleLogout = () => {
    api.logout()
    onLogout()
  }

  return (
    <div className="min-h-screen bg-background">
      {/* Header */}
      <div className="bg-surface border-b border-border">
        <div className="max-w-7xl mx-auto px-4 py-4 flex justify-between items-center">
          <h1 className="text-2xl font-bold text-text-primary">FORGE</h1>
          <div className="flex items-center gap-4">
            <button
              onClick={() => navigate('/programs')}
              className="text-text-secondary hover:text-primary transition-colors"
            >
              Programs
            </button>
            <button
              onClick={() => navigate('/workout')}
              className="px-4 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
            >
              Start Workout
            </button>
            <button
              onClick={handleLogout}
              className="text-text-secondary hover:text-danger transition-colors"
            >
              Log Out
            </button>
          </div>
        </div>
      </div>

      <div className="max-w-7xl mx-auto px-4 py-8">
        {/* Coaching State Banner */}
        {coachingState && coachingState.mode === 'ai_coaching' && (
          <div className="bg-primary bg-opacity-10 border border-primary rounded-lg p-4 mb-8">
            <div className="flex items-center justify-between">
              <div>
                <h3 className="font-bold text-text-primary">AI Coaching Mode Active</h3>
                <p className="text-sm text-text-secondary">
                  Week {coachingState.current_week} of your program
                </p>
              </div>
              <button
                onClick={() => navigate('/ai-coaching')}
                className="px-4 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
              >
                View Program
              </button>
            </div>
          </div>
        )}

        {!coachingState || coachingState.mode === 'tracker' && (
          <div className="bg-surface border border-border rounded-lg p-6 mb-8">
            <h3 className="text-xl font-bold text-text-primary mb-2">
              Ready to level up your training?
            </h3>
            <p className="text-text-secondary mb-4">
              Get a personalized program with AI coaching or browse our curated programs
            </p>
            <div className="flex gap-4">
              <button
                onClick={() => navigate('/ai-coaching')}
                className="px-6 py-2 bg-primary text-white rounded-lg hover:bg-primary-dark transition-colors"
              >
                AI Coaching Setup
              </button>
              <button
                onClick={() => navigate('/programs')}
                className="px-6 py-2 bg-surface border border-border text-text-primary rounded-lg hover:border-primary transition-colors"
              >
                Browse Programs
              </button>
            </div>
          </div>
        )}

        {/* Quick Actions */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-8">
          <button
            onClick={() => navigate('/workout')}
            className="bg-surface border border-border rounded-lg p-6 text-left hover:border-primary transition-colors"
          >
            <div className="text-3xl mb-2">💪</div>
            <h3 className="font-bold text-text-primary mb-1">Start Workout</h3>
            <p className="text-sm text-text-secondary">Log your training session</p>
          </button>

          <button className="bg-surface border border-border rounded-lg p-6 text-left hover:border-primary transition-colors">
            <div className="text-3xl mb-2">🍽️</div>
            <h3 className="font-bold text-text-primary mb-1">Log Food</h3>
            <p className="text-sm text-text-secondary">Track your nutrition</p>
          </button>

          <button className="bg-surface border border-border rounded-lg p-6 text-left hover:border-primary transition-colors">
            <div className="text-3xl mb-2">📊</div>
            <h3 className="font-bold text-text-primary mb-1">Weekly Check-in</h3>
            <p className="text-sm text-text-secondary">Update your progress</p>
          </button>
        </div>

        {/* Visualization Widgets */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-8">
          <VolumeChart data={volumeData} timeframe="week" />
          <PRList records={prData} />
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <ExerciseProgressChart exerciseName={selectedExercise} data={progressData} />
          <BodyMap volumeData={volumeData} />
        </div>
      </div>
    </div>
  )
}
