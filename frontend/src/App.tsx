import { Routes, Route, Navigate } from 'react-router-dom'
import { useState, useEffect, useCallback } from 'react'
import Dashboard from './pages/Dashboard'
import Login from './pages/Login'
import Register from './pages/Register'
import Onboarding from './pages/Onboarding'
import { api } from './services/api'

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [hasProfile, setHasProfile] = useState<boolean | null>(null)
  const [isLoading, setIsLoading] = useState(true)

  const checkProfile = useCallback(async () => {
    try {
      await api.getProfile()
      setHasProfile(true)
    } catch {
      // 404 means no profile exists yet
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
    // New users never have a profile yet
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
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-2xl text-text-secondary">Loading...</div>
      </div>
    )
  }

  // Determine where authenticated users should go
  const getAuthenticatedHome = () => {
    if (hasProfile === false) {
      return <Navigate to="/onboarding" />
    }
    return <Dashboard onLogout={handleLogout} />
  }

  return (
    <Routes>
      <Route
        path="/login"
        element={
          isAuthenticated ? <Navigate to="/" /> : <Login onLogin={handleLogin} />
        }
      />
      <Route
        path="/register"
        element={
          isAuthenticated ? <Navigate to="/" /> : <Register onRegister={handleRegister} />
        }
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
      <Route
        path="/"
        element={
          isAuthenticated ? getAuthenticatedHome() : <Navigate to="/login" />
        }
      />
    </Routes>
  )
}

export default App
