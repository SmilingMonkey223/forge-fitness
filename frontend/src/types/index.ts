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
  exercise_name?: string
  muscle_group?: string
  equipment?: string
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

export interface AddSetRequest {
  exercise_id: string
  weight_kg?: number
  reps?: number
  rpe?: number
  set_type: ExerciseSet['set_type']
}

export interface AddSetResponse {
  set: ExerciseSet
  is_pr: boolean
  estimated_1rm?: number
  previous_1rm?: number
}

export interface ExerciseHistoryEntry {
  workout_id: string
  workout_date: string
  sets: {
    weight_kg: number
    reps: number
    rpe?: number
    set_type: ExerciseSet['set_type']
    is_pr: boolean
  }[]
}

export interface ApiError {
  error: {
    code: string
    message: string
  }
}

export interface MuscleGroupVolume {
  muscle_group: string
  total_sets: number
  percentage: number
}

export interface PersonalRecord {
  id: string
  exercise_name: string
  weight_kg: number
  reps: number
  pr_type: string
  value: number
  date: string
}

export interface ConsistencyPoint {
  date: string
  workout_completed: boolean
  volume_kg: number
}

export interface Streaks {
  workout: { current: number; best: number }
  logging: { current: number; best: number }
  weight: { current: number; best: number }
}

export interface CheckInRecommendation {
  summary: string
  adaptive_tdee_kcal: number
  formula_tdee_kcal: number
  recommended_calorie_target: number
  protein_target_g: number
  carbs_target_g: number
  fat_target_g: number
  calorie_adjustment: number
}

export interface FoodItem {
  name: string
  portion_grams: number
  calories: number
  protein_g: number
  carbs_g: number
  fat_g: number
}

export interface FoodRecognitionResult {
  task_id: string
  status: 'pending' | 'processing' | 'completed' | 'failed'
  confidence?: number
  tier?: number
  items?: FoodItem[]
  error?: string
}

export interface WeightData {
  id: string
  weight_kg: number
  logged_at: string
  notes?: string
}

export interface WeightTrend {
  current_weight: number
  start_weight: number
  change_kg: number
  change_percent: number
  avg_weekly_change: number
  trend_direction: 'losing' | 'gaining' | 'stable'
  data_points: number
}

export interface WorkoutSummary {
  id: string
  name?: string
  started_at: string
  completed_at?: string
  duration_seconds?: number
  status: 'in_progress' | 'completed' | 'cancelled'
  exercises_count: number
  sets_count: number
  total_volume_kg: number
}

export interface NutritionSummary {
  date: string
  total_calories: number
  total_protein_g: number
  total_carbs_g: number
  total_fat_g: number
  meal_count: number
}

export interface CustomFood {
  id: string
  name: string
  brand?: string
  serving_size: number
  serving_unit: string
  calories: number
  protein_g: number
  carbs_g: number
  fat_g: number
}

export interface RecentFood {
  food_name: string
  brand?: string
  serving_size: number
  serving_unit: string
  calories: number
  protein_g: number
  carbs_g: number
  fat_g: number
  last_logged: string
}

export interface USDAFood {
  fdc_id: number
  description: string
  brand_owner?: string
  serving_size?: number
  serving_unit?: string
  calories: number
  protein_g: number
  carbs_g: number
  fat_g: number
}

export interface RoutineSetTemplate {
  id: string
  set_order: number
  set_type: string
  target_reps?: number
  target_weight_kg?: number
  target_rpe?: number
  rest_seconds?: number
}

export interface RoutineExercise {
  id: string
  exercise_id: string
  exercise_name: string
  order_index: number
  notes?: string
  sets: RoutineSetTemplate[]
}

export interface Routine {
  id: string
  user_id: string
  name: string
  description?: string
  exercises: RoutineExercise[]
}

export interface RoutineSummary {
  id: string
  name: string
  description?: string
  exercise_count: number
}

export interface CreateRoutineRequest {
  name: string
  description?: string
  exercises: {
    exercise_id: string
    notes?: string
    sets: {
      set_type?: string
      target_reps?: number
      target_weight_kg?: number
      target_rpe?: number
      rest_seconds?: number
    }[]
  }[]
}

export type UpdateRoutineRequest = CreateRoutineRequest
