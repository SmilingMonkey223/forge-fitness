import { create } from 'zustand'
import { persist } from 'zustand/middleware'
import type { ExerciseSet, Exercise } from '@/types'

interface ActiveExercise {
  exercise: Exercise
  sets: ExerciseSet[]
}

interface WorkoutStore {
  // Workout state
  activeWorkoutId: string | null
  workoutName: string
  currentExerciseIndex: number
  exercises: ActiveExercise[]
  elapsedSeconds: number

  // Rest timer state
  restTimerSeconds: number
  restTimerTotal: number
  restTimerRunning: boolean

  // Offline queue
  pendingSets: {
    exerciseId: string
    data: {
      weight_kg?: number
      reps?: number
      rpe?: number
      set_type: ExerciseSet['set_type']
    }
  }[]

  // Actions
  startWorkout: (id: string, name?: string) => void
  setWorkoutName: (name: string) => void
  addExercise: (exercise: Exercise) => void
  removeExercise: (index: number) => void
  setCurrentExerciseIndex: (index: number) => void
  addSet: (exerciseId: string, set: ExerciseSet) => void
  tickElapsed: () => void
  startRestTimer: (seconds: number) => void
  adjustRestTimer: (delta: number) => void
  tickRestTimer: () => void
  stopRestTimer: () => void
  addPendingSet: (exerciseId: string, data: WorkoutStore['pendingSets'][0]['data']) => void
  clearPendingSet: (index: number) => void
  finishWorkout: () => void
}

const REST_DEFAULTS: Record<string, number> = {
  warmup: 60,
  working: 90,
  drop_set: 60,
  failure: 180,
}

export const getDefaultRest = (setType: string): number =>
  REST_DEFAULTS[setType] ?? 90

export const useWorkoutStore = create<WorkoutStore>()(
  persist(
    (set) => ({
      activeWorkoutId: null,
      workoutName: 'Workout',
      currentExerciseIndex: 0,
      exercises: [],
      elapsedSeconds: 0,
      restTimerSeconds: 0,
      restTimerTotal: 0,
      restTimerRunning: false,
      pendingSets: [],

      startWorkout: (id, name) =>
        set({
          activeWorkoutId: id,
          workoutName: name || 'Workout',
          currentExerciseIndex: 0,
          exercises: [],
          elapsedSeconds: 0,
          restTimerSeconds: 0,
          restTimerTotal: 0,
          restTimerRunning: false,
          pendingSets: [],
        }),

      setWorkoutName: (name) => set({ workoutName: name }),

      addExercise: (exercise) =>
        set((state) => ({
          exercises: [...state.exercises, { exercise, sets: [] }],
          currentExerciseIndex: state.exercises.length,
        })),

      removeExercise: (index) =>
        set((state) => {
          const exercises = state.exercises.filter((_, i) => i !== index)
          return {
            exercises,
            currentExerciseIndex: Math.min(
              state.currentExerciseIndex,
              Math.max(0, exercises.length - 1)
            ),
          }
        }),

      setCurrentExerciseIndex: (index) => set({ currentExerciseIndex: index }),

      addSet: (exerciseId, newSet) =>
        set((state) => ({
          exercises: state.exercises.map((ex) =>
            ex.exercise.id === exerciseId
              ? { ...ex, sets: [...ex.sets, newSet] }
              : ex
          ),
        })),

      tickElapsed: () =>
        set((state) => ({ elapsedSeconds: state.elapsedSeconds + 1 })),

      startRestTimer: (seconds) =>
        set({
          restTimerSeconds: seconds,
          restTimerTotal: seconds,
          restTimerRunning: true,
        }),

      adjustRestTimer: (delta) =>
        set((state) => {
          const newSeconds = Math.max(0, state.restTimerSeconds + delta)
          return {
            restTimerSeconds: newSeconds,
            restTimerTotal: Math.max(state.restTimerTotal, newSeconds),
          }
        }),

      tickRestTimer: () =>
        set((state) => {
          if (state.restTimerSeconds <= 1) {
            return { restTimerSeconds: 0, restTimerRunning: false }
          }
          return { restTimerSeconds: state.restTimerSeconds - 1 }
        }),

      stopRestTimer: () =>
        set({ restTimerSeconds: 0, restTimerRunning: false }),

      addPendingSet: (exerciseId, data) =>
        set((state) => ({
          pendingSets: [...state.pendingSets, { exerciseId, data }],
        })),

      clearPendingSet: (index) =>
        set((state) => ({
          pendingSets: state.pendingSets.filter((_, i) => i !== index),
        })),

      finishWorkout: () =>
        set({
          activeWorkoutId: null,
          workoutName: 'Workout',
          currentExerciseIndex: 0,
          exercises: [],
          elapsedSeconds: 0,
          restTimerSeconds: 0,
          restTimerTotal: 0,
          restTimerRunning: false,
          pendingSets: [],
        }),
    }),
    {
      name: 'forge-workout',
    }
  )
)
