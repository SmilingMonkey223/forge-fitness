import { Routes, Route, Navigate } from 'react-router-dom'
import { useState, useEffect } from 'react'
import Dashboard from './pages/Dashboard'
import { EnhancedDashboard } from './pages/EnhancedDashboard'
import Login from './pages/Login'
import Register from './pages/Register'
import { WorkoutLogger } from './pages/WorkoutLogger'
import { ProgramBrowser } from './pages/ProgramBrowser'
import { AICoachingSetup } from './pages/AICoachingSetup'
import { Feed } from './pages/Feed'
import { api } from './services/api'

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    // Check if user has a valid token
    const token = api.getAccessToken()
    if (token) {
      setIsAuthenticated(true)
    }
    setIsLoading(false)
  }, [])

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-2xl text-text-secondary">Loading...</div>
      </div>
    )
  }

  return (
    <Routes>
      <Route
        path="/login"
        element={
          isAuthenticated ? <Navigate to="/" /> : <Login onLogin={() => setIsAuthenticated(true)} />
        }
      />
      <Route
        path="/register"
        element={
          isAuthenticated ? <Navigate to="/" /> : <Register onRegister={() => setIsAuthenticated(true)} />
        }
      />
      <Route
        path="/"
        element={
          isAuthenticated ? <EnhancedDashboard onLogout={() => setIsAuthenticated(false)} /> : <Navigate to="/login" />
        }
      />
      <Route
        path="/dashboard-simple"
        element={
          isAuthenticated ? <Dashboard onLogout={() => setIsAuthenticated(false)} /> : <Navigate to="/login" />
        }
      />
      <Route
        path="/workout"
        element={
          isAuthenticated ? <WorkoutLogger /> : <Navigate to="/login" />
        }
      />
      <Route
        path="/programs"
        element={
          isAuthenticated ? <ProgramBrowser /> : <Navigate to="/login" />
        }
      />
      <Route
        path="/ai-coaching"
        element={
          isAuthenticated ? <AICoachingSetup /> : <Navigate to="/login" />
        }
      />
      <Route
        path="/feed"
        element={
          isAuthenticated ? <Feed onLogout={() => setIsAuthenticated(false)} /> : <Navigate to="/login" />
        }
      />
    </Routes>
  )
}

export default App
