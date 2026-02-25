import type {
  AuthResponse,
  UserProfile,
  DashboardData,
  ApiError,
  Workout,
  Exercise,
  ExerciseSet,
  NutritionLog,
  AddSetRequest,
  AddSetResponse,
  ExerciseHistoryEntry,
  MuscleGroupVolume,
  PersonalRecord,
  ConsistencyPoint,
  Streaks,
  CheckInRecommendation,
  FoodItem,
  FoodRecognitionResult,
  WeightData,
  WeightTrend,
  WorkoutSummary,
  NutritionSummary,
  CustomFood,
  RecentFood,
  USDAFood,
  Routine,
  RoutineSummary,
  CreateRoutineRequest,
  UpdateRoutineRequest,
} from '@/types'

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
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      ...(options.headers as Record<string, string> || {}),
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

  async updateProfile(data: Partial<UserProfile>): Promise<{ profile: UserProfile }> {
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

  // Workout endpoints
  async createWorkout(name?: string): Promise<Workout> {
    return this.request<Workout>('/api/workouts', {
      method: 'POST',
      body: JSON.stringify({ name: name || 'Workout' }),
    })
  }

  async getWorkouts(params?: { limit?: number; offset?: number }): Promise<WorkoutSummary[]> {
    const query = new URLSearchParams()
    if (params?.limit) query.set('limit', String(params.limit))
    if (params?.offset) query.set('offset', String(params.offset))
    const qs = query.toString()
    return this.request<WorkoutSummary[]>(`/api/workouts${qs ? `?${qs}` : ''}`)
  }

  async getWorkout(workoutId: string): Promise<Workout> {
    return this.request<Workout>(`/api/workouts/${workoutId}`)
  }

  async addSet(workoutId: string, data: AddSetRequest): Promise<AddSetResponse> {
    return this.request<AddSetResponse>(`/api/workouts/${workoutId}/sets`, {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  async completeWorkout(workoutId: string): Promise<Workout> {
    return this.request<Workout>(`/api/workouts/${workoutId}/complete`, {
      method: 'PUT',
    })
  }

  async deleteWorkout(workoutId: string): Promise<void> {
    return this.request<void>(`/api/workouts/${workoutId}`, {
      method: 'DELETE',
    })
  }

  async updateSet(workoutId: string, setId: string, data: Partial<ExerciseSet>): Promise<ExerciseSet> {
    return this.request<ExerciseSet>(`/api/workouts/${workoutId}/sets/${setId}`, {
      method: 'PUT',
      body: JSON.stringify(data),
    })
  }

  async deleteSet(workoutId: string, setId: string): Promise<void> {
    return this.request<void>(`/api/workouts/${workoutId}/sets/${setId}`, {
      method: 'DELETE',
    })
  }

  // Exercise endpoints
  async getExercises(params?: {
    search?: string
    muscle_group?: string
  }): Promise<Exercise[]> {
    const query = new URLSearchParams()
    if (params?.search) query.set('search', params.search)
    if (params?.muscle_group) query.set('muscle_group', params.muscle_group)
    const qs = query.toString()
    return this.request<Exercise[]>(`/api/exercises${qs ? `?${qs}` : ''}`)
  }

  async getExercise(exerciseId: string): Promise<Exercise> {
    return this.request<Exercise>(`/api/exercises/${exerciseId}`)
  }

  async getExerciseHistory(exerciseId: string): Promise<ExerciseHistoryEntry[]> {
    return this.request<ExerciseHistoryEntry[]>(
      `/api/exercises/${exerciseId}/history`
    )
  }

  // Nutrition endpoints
  async logNutrition(data: {
    meal_type?: string
    food_name: string
    serving_size?: number
    serving_unit?: string
    quantity?: number
    calories: number
    protein_g: number
    carbs_g: number
    fat_g: number
  }): Promise<NutritionLog> {
    return this.request<NutritionLog>('/api/nutrition/log', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  async getNutritionLog(date: string): Promise<NutritionLog[]> {
    return this.request<NutritionLog[]>(`/api/nutrition/log?date=${date}`)
  }

  async updateNutritionLog(id: string, data: Partial<NutritionLog>): Promise<NutritionLog> {
    return this.request<NutritionLog>(`/api/nutrition/log/${id}`, {
      method: 'PUT',
      body: JSON.stringify(data),
    })
  }

  async deleteNutritionLog(id: string): Promise<void> {
    return this.request<void>(`/api/nutrition/log/${id}`, {
      method: 'DELETE',
    })
  }

  async getNutritionSummary(date: string): Promise<NutritionSummary> {
    return this.request<NutritionSummary>(`/api/nutrition/summary?date=${date}`)
  }

  async getNutritionSummaryRange(start: string, end: string): Promise<NutritionSummary[]> {
    return this.request<NutritionSummary[]>(`/api/nutrition/summary/range?start=${start}&end=${end}`)
  }

  async createCustomFood(data: Omit<CustomFood, 'id'>): Promise<CustomFood> {
    return this.request<CustomFood>('/api/nutrition/foods', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  async getCustomFoods(): Promise<CustomFood[]> {
    return this.request<CustomFood[]>('/api/nutrition/foods')
  }

  async deleteCustomFood(id: string): Promise<void> {
    return this.request<void>(`/api/nutrition/foods/${id}`, {
      method: 'DELETE',
    })
  }

  async getRecentFoods(): Promise<RecentFood[]> {
    return this.request<RecentFood[]>('/api/nutrition/recent')
  }

  async searchFoods(query: string): Promise<USDAFood[]> {
    return this.request<USDAFood[]>(`/api/nutrition/search?q=${encodeURIComponent(query)}`)
  }

  // Weight endpoints
  async logWeight(weight_kg: number): Promise<{ new_badges?: { code: string; name: string }[] }> {
    return this.request<{ new_badges?: { code: string; name: string }[] }>('/api/weight', {
      method: 'POST',
      body: JSON.stringify({ weight_kg }),
    })
  }

  async getWeightHistory(): Promise<{ data: WeightData[] }> {
    return this.request<{ data: WeightData[] }>('/api/weight')
  }

  async getWeightTrend(): Promise<WeightTrend> {
    return this.request<WeightTrend>('/api/weight/trend')
  }

  // Analytics endpoints
  async getMuscleDistribution(): Promise<{ data: MuscleGroupVolume[] }> {
    return this.request<{ data: MuscleGroupVolume[] }>('/api/analytics/muscle-distribution')
  }

  async getPRHistory(): Promise<{ prs: PersonalRecord[] }> {
    return this.request<{ prs: PersonalRecord[] }>('/api/analytics/prs')
  }

  async getTrainingConsistency(): Promise<{ data: ConsistencyPoint[] }> {
    return this.request<{ data: ConsistencyPoint[] }>('/api/analytics/consistency')
  }

  async getStreaks(): Promise<Streaks> {
    return this.request<Streaks>('/api/analytics/streaks')
  }

  async generateNutritionCheckin(): Promise<CheckInRecommendation> {
    return this.request<CheckInRecommendation>('/api/analytics/nutrition-checkin', {
      method: 'POST',
    })
  }

  // Food recognition endpoints
  async recognizeFood(image: Blob): Promise<{ task_id: string }> {
    const formData = new FormData()
    formData.append('image', image)

    const token = this.getAccessToken()
    const headers: Record<string, string> = {}
    if (token) {
      headers['Authorization'] = `Bearer ${token}`
    }

    const response = await fetch(`${API_BASE}/api/nutrition/recognize`, {
      method: 'POST',
      headers,
      body: formData,
    })

    if (!response.ok) {
      const error: ApiError = await response.json()
      throw new Error(error.error.code || 'REQUEST_FAILED')
    }

    return response.json()
  }

  async getFoodRecognitionResult(taskId: string): Promise<FoodRecognitionResult> {
    return this.request<FoodRecognitionResult>(`/api/nutrition/recognize/${taskId}`)
  }

  async confirmFoodRecognition(taskId: string, items: FoodItem[], mealType: string): Promise<void> {
    return this.request<void>(`/api/nutrition/recognize/${taskId}/confirm`, {
      method: 'POST',
      body: JSON.stringify({ items, meal_type: mealType }),
    })
  }

  // Weekly check-in endpoint
  async submitWeeklyCheckin(data: {
    hunger_rating: number
    energy_rating: number
    recovery_rating: number
    notes?: string
  }): Promise<void> {
    return this.request<void>('/api/coaching/weekly-checkin', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  // Routine endpoints
  async getRoutines(): Promise<RoutineSummary[]> {
    return this.request<RoutineSummary[]>('/api/routines')
  }

  async getRoutine(id: string): Promise<Routine> {
    return this.request<Routine>(`/api/routines/${id}`)
  }

  async createRoutine(data: CreateRoutineRequest): Promise<Routine> {
    return this.request<Routine>('/api/routines', {
      method: 'POST',
      body: JSON.stringify(data),
    })
  }

  async updateRoutine(id: string, data: UpdateRoutineRequest): Promise<Routine> {
    return this.request<Routine>(`/api/routines/${id}`, {
      method: 'PUT',
      body: JSON.stringify(data),
    })
  }

  async deleteRoutine(id: string): Promise<void> {
    return this.request<void>(`/api/routines/${id}`, {
      method: 'DELETE',
    })
  }

  async startWorkoutFromRoutine(routineId: string): Promise<Workout> {
    return this.request<Workout>(`/api/routines/${routineId}/start`, {
      method: 'POST',
    })
  }

  // Health check
  async health(): Promise<{ status: string }> {
    return this.request<{ status: string }>('/health')
  }
}

export const api = new ApiClient()
