import { useState, useEffect, useRef, useCallback } from 'react'
import { api } from '@/services/api'
import type { Exercise } from '@/types'

const MUSCLE_GROUPS = ['All', 'Chest', 'Back', 'Shoulders', 'Biceps', 'Triceps', 'Legs', 'Core']

interface ExercisePickerProps {
  open: boolean
  onSelect: (exercise: Exercise) => void
  onClose: () => void
}

export default function ExercisePicker({ open, onSelect, onClose }: ExercisePickerProps) {
  const [search, setSearch] = useState('')
  const [muscleGroup, setMuscleGroup] = useState('All')
  const [exercises, setExercises] = useState<Exercise[]>([])
  const [loading, setLoading] = useState(false)
  const debounceRef = useRef<ReturnType<typeof setTimeout> | null>(null)
  const inputRef = useRef<HTMLInputElement>(null)

  const fetchExercises = useCallback(async (searchTerm: string, group: string) => {
    setLoading(true)
    try {
      const params: { search?: string; muscle_group?: string } = {}
      if (searchTerm) params.search = searchTerm
      if (group !== 'All') params.muscle_group = group.toLowerCase()
      const results = await api.getExercises(params)
      setExercises(results)
    } catch {
      setExercises([])
    } finally {
      setLoading(false)
    }
  }, [])

  // Fetch on open
  useEffect(() => {
    if (open) {
      fetchExercises('', 'All')
      setSearch('')
      setMuscleGroup('All')
      // Focus search input
      setTimeout(() => inputRef.current?.focus(), 100)
    }
  }, [open, fetchExercises])

  // Debounced search
  useEffect(() => {
    if (!open) return
    if (debounceRef.current) clearTimeout(debounceRef.current)
    debounceRef.current = setTimeout(() => {
      fetchExercises(search, muscleGroup)
    }, 300)
    return () => {
      if (debounceRef.current) clearTimeout(debounceRef.current)
    }
  }, [search, muscleGroup, open, fetchExercises])

  if (!open) return null

  return (
    <div className="fixed inset-0 z-40 flex flex-col bg-background">
      {/* Header */}
      <div className="flex items-center gap-3 px-4 py-3 bg-surface border-b border-text-muted">
        <button
          onClick={onClose}
          className="tap-target p-2 text-text-secondary hover:text-text-primary transition-colors"
        >
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
        </button>
        <h2 className="text-lg font-bold flex-1">Add Exercise</h2>
      </div>

      {/* Search */}
      <div className="px-4 py-3">
        <input
          ref={inputRef}
          type="text"
          placeholder="Search exercises..."
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          className="input"
        />
      </div>

      {/* Filter chips */}
      <div className="px-4 pb-3 flex gap-2 overflow-x-auto no-scrollbar">
        {MUSCLE_GROUPS.map((group) => (
          <button
            key={group}
            onClick={() => setMuscleGroup(group)}
            className={`tap-target px-4 py-2 rounded-button text-sm font-medium whitespace-nowrap transition-all ${
              muscleGroup === group
                ? 'bg-primary text-white'
                : 'bg-surface-elevated text-text-secondary hover:text-text-primary'
            }`}
          >
            {group}
          </button>
        ))}
      </div>

      {/* Exercise list */}
      <div className="flex-1 overflow-y-auto px-4">
        {loading ? (
          <div className="flex items-center justify-center py-12">
            <div className="text-text-secondary">Loading exercises...</div>
          </div>
        ) : exercises.length === 0 ? (
          <div className="flex items-center justify-center py-12">
            <div className="text-text-muted">No exercises found</div>
          </div>
        ) : (
          <div className="space-y-1">
            {exercises.map((exercise) => (
              <button
                key={exercise.id}
                onClick={() => onSelect(exercise)}
                className="tap-target w-full flex items-center justify-between px-4 py-3
                           rounded-lg hover:bg-surface-elevated active:bg-surface transition-colors text-left"
              >
                <div>
                  <div className="font-medium text-text-primary">{exercise.name}</div>
                  <div className="text-sm text-text-muted capitalize">{exercise.muscle_group}</div>
                </div>
                <span className="text-xs px-2 py-1 rounded bg-surface-elevated text-text-secondary capitalize">
                  {exercise.equipment}
                </span>
              </button>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}
