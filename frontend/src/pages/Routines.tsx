import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useQuery, useQueryClient } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { RoutineSummary, Routine, Exercise, CreateRoutineRequest } from '@/types'
import { useWorkoutStore } from '@/stores/workoutStore'
import ExercisePicker from '@/components/ExercisePicker'

// --- Routine Editor ---

interface RoutineEditorProps {
  initial?: Routine
  onSave: (data: CreateRoutineRequest) => Promise<void>
  onCancel: () => void
}

interface EditableExercise {
  exercise_id: string
  exercise_name: string
  sets: { set_type: string; target_reps?: number; target_weight_kg?: number; rest_seconds?: number }[]
}

function RoutineEditor({ initial, onSave, onCancel }: RoutineEditorProps) {
  const [name, setName] = useState(initial?.name || '')
  const [description, setDescription] = useState(initial?.description || '')
  const [exercises, setExercises] = useState<EditableExercise[]>(
    initial?.exercises.map((e) => ({
      exercise_id: e.exercise_id,
      exercise_name: e.exercise_name,
      sets: e.sets.map((s) => ({
        set_type: s.set_type,
        target_reps: s.target_reps,
        target_weight_kg: s.target_weight_kg,
        rest_seconds: s.rest_seconds,
      })),
    })) || []
  )
  const [showPicker, setShowPicker] = useState(false)
  const [saving, setSaving] = useState(false)
  const [editorMessage, setEditorMessage] = useState<{ type: 'error' | 'success'; text: string } | null>(null)

  const addExercise = (exercise: Exercise) => {
    setExercises([
      ...exercises,
      {
        exercise_id: exercise.id,
        exercise_name: exercise.name,
        sets: [{ set_type: 'working', target_reps: 10 }],
      },
    ])
    setShowPicker(false)
  }

  const removeExercise = (index: number) => {
    setExercises(exercises.filter((_, i) => i !== index))
  }

  const moveExercise = (index: number, direction: -1 | 1) => {
    const newIndex = index + direction
    if (newIndex < 0 || newIndex >= exercises.length) return
    const copy = [...exercises]
    ;[copy[index], copy[newIndex]] = [copy[newIndex], copy[index]]
    setExercises(copy)
  }

  const addSet = (exIndex: number) => {
    setExercises(
      exercises.map((ex, i) =>
        i === exIndex
          ? { ...ex, sets: [...ex.sets, { set_type: 'working', target_reps: 10 }] }
          : ex
      )
    )
  }

  const removeSet = (exIndex: number, setIndex: number) => {
    setExercises(
      exercises.map((ex, i) =>
        i === exIndex ? { ...ex, sets: ex.sets.filter((_, si) => si !== setIndex) } : ex
      )
    )
  }

  const updateSet = (exIndex: number, setIndex: number, field: string, value: string | number) => {
    setExercises(
      exercises.map((ex, i) =>
        i === exIndex
          ? {
              ...ex,
              sets: ex.sets.map((s, si) => (si === setIndex ? { ...s, [field]: value } : s)),
            }
          : ex
      )
    )
  }

  const handleSave = async () => {
    setEditorMessage(null)
    if (!name.trim()) {
      setEditorMessage({ type: 'error', text: 'Routine name is required' })
      return
    }
    if (exercises.length === 0) {
      setEditorMessage({ type: 'error', text: 'Add at least one exercise' })
      return
    }
    setSaving(true)
    try {
      await onSave({
        name: name.trim(),
        description: description.trim() || undefined,
        exercises: exercises.map((ex) => ({
          exercise_id: ex.exercise_id,
          sets: ex.sets.map((s) => ({
            set_type: s.set_type,
            target_reps: s.target_reps,
            target_weight_kg: s.target_weight_kg,
            rest_seconds: s.rest_seconds,
          })),
        })),
      })
    } catch {
      setEditorMessage({ type: 'error', text: 'Failed to save routine' })
    } finally {
      setSaving(false)
    }
  }

  return (
    <div className="fixed inset-0 z-40 flex flex-col bg-background overflow-y-auto">
      <ExercisePicker open={showPicker} onSelect={addExercise} onClose={() => setShowPicker(false)} />

      <div className="flex items-center gap-3 px-4 py-3 bg-surface border-b border-text-muted">
        <button onClick={onCancel} className="tap-target p-2 text-text-secondary hover:text-text-primary">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
        </button>
        <h2 className="text-lg font-bold flex-1">{initial ? 'Edit Routine' : 'New Routine'}</h2>
        <button onClick={handleSave} disabled={saving} className="btn-primary text-sm px-4 py-2">
          {saving ? 'Saving...' : 'Save'}
        </button>
      </div>

      <div className="max-w-2xl mx-auto w-full px-4 py-4 space-y-4">
        {editorMessage && (
          <div
            className={`rounded-lg p-3 text-sm text-center ${
              editorMessage.type === 'error'
                ? 'bg-danger/10 border border-danger text-danger'
                : 'bg-success/10 border border-success text-success'
            }`}
          >
            {editorMessage.text}
          </div>
        )}

        <div>
          <label className="label">Routine Name</label>
          <input
            type="text"
            value={name}
            onChange={(e) => setName(e.target.value)}
            placeholder="e.g., Push Day"
            className="input"
          />
        </div>
        <div>
          <label className="label">Description (optional)</label>
          <input
            type="text"
            value={description}
            onChange={(e) => setDescription(e.target.value)}
            placeholder="Chest, shoulders, triceps"
            className="input"
          />
        </div>

        {/* Exercises */}
        {exercises.map((ex, exIdx) => (
          <div key={exIdx} className="card p-4">
            <div className="flex items-center justify-between mb-3">
              <div className="font-bold text-primary">{ex.exercise_name}</div>
              <div className="flex gap-1">
                <button onClick={() => moveExercise(exIdx, -1)} className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-text-primary text-sm">^</button>
                <button onClick={() => moveExercise(exIdx, 1)} className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-text-primary text-sm rotate-180">^</button>
                <button onClick={() => removeExercise(exIdx)} className="tap-target w-8 h-8 flex items-center justify-center text-danger/60 hover:text-danger text-sm">X</button>
              </div>
            </div>

            {/* Set headers */}
            <div className="grid grid-cols-[40px_1fr_1fr_1fr_32px] gap-2 mb-1 px-1">
              <div className="text-xs text-text-muted text-center">Set</div>
              <div className="text-xs text-text-muted text-center">Reps</div>
              <div className="text-xs text-text-muted text-center">Weight (kg)</div>
              <div className="text-xs text-text-muted text-center">Rest (s)</div>
              <div />
            </div>

            {ex.sets.map((set, setIdx) => (
              <div key={setIdx} className="grid grid-cols-[40px_1fr_1fr_1fr_32px] gap-2 items-center py-1 px-1">
                <button
                  onClick={() => {
                    const types = ['working', 'warmup', 'drop_set']
                    const next = types[(types.indexOf(set.set_type) + 1) % types.length]
                    updateSet(exIdx, setIdx, 'set_type', next)
                  }}
                  className={`text-xs font-bold text-center tap-target rounded ${
                    set.set_type === 'warmup' ? 'text-warning' : set.set_type === 'drop_set' ? 'text-danger' : 'text-text-secondary'
                  }`}
                >
                  {set.set_type === 'warmup' ? 'W' : set.set_type === 'drop_set' ? 'D' : setIdx + 1}
                </button>
                <input
                  type="number"
                  value={set.target_reps || ''}
                  onChange={(e) => updateSet(exIdx, setIdx, 'target_reps', Number(e.target.value) || 0)}
                  placeholder="10"
                  className="w-full text-center bg-surface-elevated rounded py-2 text-sm font-mono text-text-primary
                             [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none [&::-webkit-inner-spin-button]:appearance-none"
                />
                <input
                  type="number"
                  value={set.target_weight_kg || ''}
                  onChange={(e) => updateSet(exIdx, setIdx, 'target_weight_kg', Number(e.target.value) || 0)}
                  placeholder="-"
                  className="w-full text-center bg-surface-elevated rounded py-2 text-sm font-mono text-text-primary
                             [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none [&::-webkit-inner-spin-button]:appearance-none"
                />
                <input
                  type="number"
                  value={set.rest_seconds || ''}
                  onChange={(e) => updateSet(exIdx, setIdx, 'rest_seconds', Number(e.target.value) || 0)}
                  placeholder="90"
                  className="w-full text-center bg-surface-elevated rounded py-2 text-sm font-mono text-text-primary
                             [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none [&::-webkit-inner-spin-button]:appearance-none"
                />
                <button
                  onClick={() => removeSet(exIdx, setIdx)}
                  className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-danger text-xs"
                >
                  <svg width="14" height="14" viewBox="0 0 14 14" fill="none" stroke="currentColor" strokeWidth="1.5">
                    <path d="M3 3l8 8M11 3l-8 8" />
                  </svg>
                </button>
              </div>
            ))}

            <button onClick={() => addSet(exIdx)} className="w-full mt-1 py-2 text-xs text-primary">
              + Add Set
            </button>
          </div>
        ))}

        <button onClick={() => setShowPicker(true)} className="btn-secondary w-full">
          + Add Exercise
        </button>
      </div>
    </div>
  )
}

// --- Main Routines Page ---

export default function Routines() {
  const navigate = useNavigate()
  const queryClient = useQueryClient()
  const startWorkout = useWorkoutStore((s) => s.startWorkout)
  const [editing, setEditing] = useState<Routine | 'new' | null>(null)
  const [deleting, setDeleting] = useState<string | null>(null)
  const [pageMessage, setPageMessage] = useState<{ type: 'error' | 'success'; text: string } | null>(null)

  const { data: routines, isLoading, error } = useQuery<RoutineSummary[]>({
    queryKey: ['routines'],
    queryFn: () => api.getRoutines(),
  })

  const handleCreate = async (data: CreateRoutineRequest) => {
    await api.createRoutine(data)
    queryClient.invalidateQueries({ queryKey: ['routines'] })
    setEditing(null)
  }

  const handleEdit = async (id: string) => {
    const routine = await api.getRoutine(id)
    setEditing(routine)
  }

  const handleUpdate = async (data: CreateRoutineRequest) => {
    if (editing && typeof editing !== 'string') {
      await api.updateRoutine(editing.id, data)
      queryClient.invalidateQueries({ queryKey: ['routines'] })
      setEditing(null)
    }
  }

  const handleDelete = async (id: string) => {
    if (!confirm('Delete this routine?')) return
    setDeleting(id)
    setPageMessage(null)
    try {
      await api.deleteRoutine(id)
      queryClient.invalidateQueries({ queryKey: ['routines'] })
    } catch {
      setPageMessage({ type: 'error', text: 'Failed to delete routine' })
    } finally {
      setDeleting(null)
    }
  }

  const handleStartFromRoutine = async (routineId: string) => {
    setPageMessage(null)
    try {
      const workout = await api.startWorkoutFromRoutine(routineId)
      startWorkout(workout.id, workout.name)
      navigate('/workout/active')
    } catch {
      setPageMessage({ type: 'error', text: 'Failed to start workout from routine. Please try again.' })
    }
  }

  if (editing) {
    return (
      <RoutineEditor
        initial={typeof editing === 'string' ? undefined : editing}
        onSave={typeof editing === 'string' ? handleCreate : handleUpdate}
        onCancel={() => setEditing(null)}
      />
    )
  }

  return (
    <div className="min-h-screen bg-background pb-20">
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-2xl mx-auto px-4 py-4 flex items-center justify-between">
          <h1 className="text-2xl font-bold">My Routines</h1>
          <button onClick={() => setEditing('new')} className="btn-primary text-sm px-4 py-2">
            + New
          </button>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-4 space-y-4">
        {pageMessage && (
          <div
            className={`rounded-lg p-3 text-sm text-center ${
              pageMessage.type === 'error'
                ? 'bg-danger/10 border border-danger text-danger'
                : 'bg-success/10 border border-success text-success'
            }`}
          >
            {pageMessage.text}
          </div>
        )}

        {isLoading && (
          <div className="text-center py-12 text-text-muted">Loading routines...</div>
        )}

        {error && (
          <div className="card text-center py-8">
            <div className="text-danger mb-2">Failed to load routines</div>
            <button onClick={() => window.location.reload()} className="text-primary text-sm">Retry</button>
          </div>
        )}

        {!isLoading && !error && (routines || []).length === 0 && (
          <div className="card text-center py-12">
            <div className="text-text-muted mb-2">No routines yet</div>
            <div className="text-text-muted text-sm mb-4">Create a routine to quickly start workouts</div>
            <button onClick={() => setEditing('new')} className="btn-primary">
              Create Your First Routine
            </button>
          </div>
        )}

        {(routines || []).map((r) => (
          <div key={r.id} className="card">
            <div className="flex items-start justify-between mb-3">
              <div>
                <div className="font-bold text-text-primary text-lg">{r.name}</div>
                {r.description && (
                  <div className="text-sm text-text-muted mt-0.5">{r.description}</div>
                )}
                <div className="text-xs text-text-muted mt-1">{r.exercise_count} exercises</div>
              </div>
            </div>

            <div className="flex gap-2">
              <button
                onClick={() => handleStartFromRoutine(r.id)}
                className="btn-primary flex-1 text-sm py-2"
              >
                Start Workout
              </button>
              <button
                onClick={() => handleEdit(r.id)}
                className="btn-secondary text-sm px-4 py-2"
              >
                Edit
              </button>
              <button
                onClick={() => handleDelete(r.id)}
                disabled={deleting === r.id}
                className="tap-target w-10 h-10 flex items-center justify-center text-text-muted hover:text-danger transition-colors rounded-lg"
              >
                <svg width="18" height="18" viewBox="0 0 18 18" fill="none" stroke="currentColor" strokeWidth="1.5">
                  <path d="M3 5h12M7 5V3h4v2M5 5v10h8V5" />
                </svg>
              </button>
            </div>
          </div>
        ))}
      </div>
    </div>
  )
}
