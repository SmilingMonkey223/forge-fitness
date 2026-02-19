import { useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { api } from '@/services/api'

interface RegisterProps {
  onRegister: () => void
}

export default function Register({ onRegister }: RegisterProps) {
  const navigate = useNavigate()
  const [email, setEmail] = useState('')
  const [username, setUsername] = useState('')
  const [displayName, setDisplayName] = useState('')
  const [password, setPassword] = useState('')
  const [confirmPassword, setConfirmPassword] = useState('')
  const [error, setError] = useState('')
  const [isLoading, setIsLoading] = useState(false)

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')

    if (password !== confirmPassword) {
      setError('Passwords do not match')
      return
    }

    setIsLoading(true)

    try {
      await api.register({
        email,
        username,
        display_name: displayName,
        password,
      })
      onRegister()
      navigate('/')
    } catch (err: any) {
      const errorMessages: Record<string, string> = {
        EMAIL_TAKEN: 'This email is already registered',
        USERNAME_TAKEN: 'This username is already taken',
        WEAK_PASSWORD: 'Password must be at least 8 characters with uppercase, lowercase, and a number',
        INVALID_EMAIL: 'Please enter a valid email address',
      }
      setError(errorMessages[err.message] || 'Registration failed. Please try again.')
    } finally {
      setIsLoading(false)
    }
  }

  return (
    <div className="min-h-screen flex items-center justify-center px-4 py-12">
      <div className="card max-w-md w-full">
        <h1 className="text-center mb-8">Create Account</h1>

        <form onSubmit={handleSubmit} className="space-y-6">
          <div>
            <label className="label">Email</label>
            <input
              type="email"
              className="input"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              required
              autoComplete="email"
            />
          </div>

          <div>
            <label className="label">Username</label>
            <input
              type="text"
              className="input"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              required
              minLength={3}
              maxLength={24}
              pattern="[a-zA-Z0-9_]+"
              title="3-24 characters, letters, numbers, and underscores only"
            />
          </div>

          <div>
            <label className="label">Display Name</label>
            <input
              type="text"
              className="input"
              value={displayName}
              onChange={(e) => setDisplayName(e.target.value)}
              required
            />
          </div>

          <div>
            <label className="label">Password</label>
            <input
              type="password"
              className="input"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
              minLength={8}
              autoComplete="new-password"
            />
            <p className="text-xs text-text-muted mt-1">
              At least 8 characters with uppercase, lowercase, and a number
            </p>
          </div>

          <div>
            <label className="label">Confirm Password</label>
            <input
              type="password"
              className="input"
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              required
              autoComplete="new-password"
            />
          </div>

          {error && (
            <div className="bg-danger bg-opacity-10 border border-danger text-danger px-4 py-3 rounded">
              {error}
            </div>
          )}

          <button
            type="submit"
            className="btn-primary w-full"
            disabled={isLoading}
          >
            {isLoading ? 'Creating account...' : 'Create Account'}
          </button>

          <p className="text-center text-text-secondary">
            Already have an account?{' '}
            <Link to="/login" className="text-primary hover:underline">
              Log in
            </Link>
          </p>
        </form>
      </div>
    </div>
  )
}
