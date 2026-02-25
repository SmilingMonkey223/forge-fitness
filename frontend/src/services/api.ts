import type { AuthResponse, User, UserProfile, DashboardData, ApiError } from '@/types'

const API_BASE = import.meta.env.VITE_API_URL || 'http://localhost:8080'

class ApiClient {
  private accessToken: string | null = null

  setAccessToken(token: string | null) {
    this.accessToken = token
    if (token) {
      localStorage.setItem('access_token', token)
    } else {
      localStorage.removeItem('access_token')
    }
  }

  getAccessToken(): string | null {
    if (!this.accessToken) {
      this.accessToken = localStorage.getItem('access_token')
    }
    return this.accessToken
  }

  async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<T> {
    const headers: HeadersInit = {
      'Content-Type': 'application/json',
      ...options.headers,
    }

    const token = this.getAccessToken()
    if (token) {
      headers['Authorization'] = `Bearer ${token}`
    }

    const response = await fetch(`${API_BASE}${endpoint}`, {
      ...options,
      headers,
    })

    if (!response.ok) {
      const error: ApiError = await response.json()
      throw new Error(error.error.code || 'REQUEST_FAILED')
    }

    return response.json()
  }

  // Auth endpoints
  async register(data: {
    email: string
    username: string
    password: string
    display_name: string
  }): Promise<AuthResponse> {
    const response = await this.request<AuthResponse>('/api/auth/register', {
      method: 'POST',
      body: JSON.stringify(data),
    })

    this.setAccessToken(response.access_token)
    return response
  }

  async login(data: {
    email: string
    password: string
  }): Promise<AuthResponse> {
    const response = await this.request<AuthResponse>('/api/auth/login', {
      method: 'POST',
      body: JSON.stringify(data),
    })

    this.setAccessToken(response.access_token)
    return response
  }

  async logout() {
    this.setAccessToken(null)
  }

  // Profile endpoints
  async getProfile(): Promise<{ profile: UserProfile }> {
    return this.request<{ profile: UserProfile }>('/api/profile')
  }

  async updateProfile(data: Partial<Omit<UserProfile, 'id' | 'user_id' | 'tdee_calories' | 'target_calories' | 'target_protein_g' | 'target_carbs_g' | 'target_fat_g'>>): Promise<{ profile: UserProfile }> {
    return this.request<{ profile: UserProfile }>('/api/profile', {
      method: 'PUT',
      body: JSON.stringify(data),
    })
  }

  async completeOnboarding(data: {
    date_of_birth: string
    sex: string
    height_cm: number
    weight_kg: number
    activity_level: string
    fitness_goal: string
    unit_preference?: string
  }): Promise<{ profile: UserProfile }> {
    return this.request<{ profile: UserProfile }>('/api/profile/onboarding', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  // Dashboard endpoint
  async getDashboard(): Promise<DashboardData> {
    return this.request<DashboardData>('/api/dashboard')
  }

  // Health check
  async health(): Promise<{ status: string }> {
    return this.request<{ status: string }>('/health')
  }
}

export const api = new ApiClient()
