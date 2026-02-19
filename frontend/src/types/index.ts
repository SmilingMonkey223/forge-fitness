export interface User {
  id: string
  email: string
  username: string
  display_name: string
}

export interface UserProfile {
  id: string
  user_id: string
  date_of_birth: string
  sex: 'male' | 'female' | 'other'
  height_cm: number
  weight_kg: number
  activity_level: 'sedentary' | 'lightly_active' | 'moderately_active' | 'very_active' | 'extremely_active'
  fitness_goal: 'lose_fat' | 'maintain' | 'build_muscle'
  unit_preference: 'metric' | 'imperial'
  tdee_calories?: number
  target_calories?: number
  target_protein_g?: number
  target_carbs_g?: number
  target_fat_g?: number
}

export interface Exercise {
  id: string
  name: string
  muscle_group: string
  equipment: string
  is_custom: boolean
  created_by?: string
}

export interface ExerciseSet {
  id: string
  workout_id: string
  exercise_id: string
  set_order: number
  exercise_order: number
  set_type: 'warmup' | 'working' | 'drop_set' | 'failure'
  reps?: number
  weight_kg?: number
  duration_seconds?: number
  rpe?: number
  rest_seconds?: number
  is_pr: boolean
  notes?: string
}

export interface Workout {
  id: string
  user_id: string
  name?: string
  started_at: string
  completed_at?: string
  duration_seconds?: number
  notes?: string
  status: 'in_progress' | 'completed' | 'cancelled'
  sets: ExerciseSet[]
}

export interface NutritionLog {
  id: string
  user_id: string
  logged_at: string
  meal_type?: 'breakfast' | 'lunch' | 'dinner' | 'snack'
  food_name: string
  brand?: string
  serving_size: number
  serving_unit: string
  quantity: number
  calories: number
  protein_g: number
  carbs_g: number
  fat_g: number
  fiber_g?: number
  sugar_g?: number
  sodium_mg?: number
  is_custom: boolean
  source: 'manual' | 'database' | 'barcode' | 'ai'
}

export interface DashboardData {
  today: {
    date: string
    nutrition: {
      calories: { consumed: number; target: number }
      protein_g: { consumed: number; target: number }
      carbs_g: { consumed: number; target: number }
      fat_g: { consumed: number; target: number }
    }
    workout: {
      completed: boolean
      name: string
      duration_seconds: number
      total_volume_kg: number
      exercises_count: number
      sets_count: number
      prs_count: number
    } | null
  }
  week: {
    workout_days: boolean[]
    daily_calories: (number | null)[]
    daily_protein: (number | null)[]
    calorie_target: number
    protein_target: number
    current_streak: number
  }
}

export interface AuthResponse {
  user: User
  access_token: string
  refresh_token: string
}

export interface ApiError {
  error: {
    code: string
    message: string
  }
}
