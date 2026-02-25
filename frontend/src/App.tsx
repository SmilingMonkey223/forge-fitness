import { Routes, Route, Navigate } from 'react-router-dom'
import { useState, useEffect, useCallback } from 'react'
import Dashboard from './pages/Dashboard'
import Login from './pages/Login'
import Register from './pages/Register'
import Onboarding from './pages/Onboarding'
import Analytics from './pages/Analytics'
import WeightTracker from './pages/WeightTracker'
import FoodRecognition from './pages/FoodRecognition'
import { NutritionLogger } from './pages/NutritionLogger'
import { WeeklyCheckIn } from './pages/WeeklyCheckIn'
import ActiveWorkout from './pages/ActiveWorkout'
import WorkoutHistory from './pages/WorkoutHistory'
import Routines from './pages/Routines'
import Settings from './pages/Settings'
import BottomNav from './components/BottomNav'
import { api } from './services/api'

function AuthenticatedLayout({ children }: { children: React.ReactNode }) {
  return (
    <div className="min-h-screen bg-background text-text-primary pb-16">
      {children}
      <BottomNav />
    </div>
  )
}

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [hasProfile, setHasProfile] = useState<boolean | null>(null)
  const [isLoading, setIsLoading] = useState(true)

  const checkProfile = useCallback(async () => {
    try {
      await api.getProfile()
      setHasProfile(true)
    } catch {
      setHasProfile(false)
    }
  }, [])

  useEffect(() => {
    const init = async () => {
      const token = api.getAccessToken()
      if (token) {
        setIsAuthenticated(true)
        await checkProfile()
      }
      setIsLoading(false)
    }
    init()
  }, [checkProfile])

  const handleLogin = async () => {
    setIsAuthenticated(true)
    await checkProfile()
  }

  const handleRegister = async () => {
    setIsAuthenticated(true)
    setHasProfile(false)
  }

  const handleLogout = () => {
    setIsAuthenticated(false)
    setHasProfile(null)
    api.logout()
  }

  const handleOnboardingComplete = () => {
    setHasProfile(true)
  }

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-background">
        <div className="text-text-secondary animate-pulse">Loading...</div>
      </div>
    )
  }

  const getAuthenticatedHome = () => {
    if (hasProfile === false) {
      return <Navigate to="/onboarding" />
    }
    return (
      <AuthenticatedLayout>
        <Dashboard onLogout={handleLogout} />
      </AuthenticatedLayout>
    )
  }

  const requireAuth = (element: React.ReactNode) =>
    isAuthenticated ? element : <Navigate to="/login" />

  const withLayout = (element: React.ReactNode) =>
    requireAuth(<AuthenticatedLayout>{element}</AuthenticatedLayout>)

  return (
    <Routes>
      <Route
        path="/login"
        element={isAuthenticated ? <Navigate to="/" /> : <Login onLogin={handleLogin} />}
      />
      <Route
        path="/register"
        element={isAuthenticated ? <Navigate to="/" /> : <Register onRegister={handleRegister} />}
      />
      <Route
        path="/onboarding"
        element={
          !isAuthenticated ? (
            <Navigate to="/login" />
          ) : hasProfile ? (
            <Navigate to="/" />
          ) : (
            <Onboarding onComplete={handleOnboardingComplete} />
          )
        }
      />
      <Route path="/" element={isAuthenticated ? getAuthenticatedHome() : <Navigate to="/login" />} />
      <Route path="/analytics" element={withLayout(<Analytics />)} />
      <Route path="/weight" element={withLayout(<WeightTracker />)} />
      <Route path="/nutrition" element={withLayout(<NutritionLogger />)} />
      <Route path="/food-recognition" element={withLayout(<FoodRecognition />)} />
      <Route path="/weekly-checkin" element={withLayout(<WeeklyCheckIn />)} />
      <Route path="/workout/active" element={requireAuth(<ActiveWorkout />)} />
      <Route path="/workout/history" element={withLayout(<WorkoutHistory />)} />
      <Route path="/routines" element={withLayout(<Routines />)} />
      <Route path="/settings" element={withLayout(<Settings onLogout={handleLogout} />)} />
      <Route path="/workout" element={withLayout(<Dashboard onLogout={handleLogout} />)} />
    </Routes>
  )
}

export default App
