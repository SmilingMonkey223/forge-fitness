import { useState, useEffect, useRef, useCallback } from 'react'
import { useNavigate } from 'react-router-dom'
import { useWorkoutStore, getDefaultRest } from '@/stores/workoutStore'
import { api } from '@/services/api'
import type { ExerciseSet, Exercise, ExerciseHistoryEntry } from '@/types'
import RestTimer from '@/components/RestTimer'
import ExercisePicker from '@/components/ExercisePicker'

// --- Set Input Row ---

interface SetInputRowProps {
  setNumber: number
  previous?: string
  onConfirm: (data: {
    weight_kg: number
    reps: number
    rpe?: number
    set_type: ExerciseSet['set_type']
  }) => void
  defaultWeight?: number
  defaultReps?: number
}

function SetInputRow({ setNumber, previous, onConfirm, defaultWeight, defaultReps }: SetInputRowProps) {
  const [weight, setWeight] = useState(defaultWeight ?? 0)
  const [reps, setReps] = useState(defaultReps ?? 0)
  const [rpe, setRpe] = useState<number | undefined>(undefined)
  const [setType, setSetType] = useState<ExerciseSet['set_type']>('working')
  const [confirmed, setConfirmed] = useState(false)
  const [flash, setFlash] = useState(false)

  const handleConfirm = () => {
    if (confirmed || (weight === 0 && reps === 0)) return
    setConfirmed(true)
    setFlash(true)
    setTimeout(() => setFlash(false), 600)
    onConfirm({ weight_kg: weight, reps, rpe, set_type: setType })
  }

  const setTypes: ExerciseSet['set_type'][] = ['working', 'warmup', 'drop_set', 'failure']

  const rpeValues = [6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10]

  return (
    <div className={`rounded-lg transition-colors duration-600 ${
      flash ? 'bg-success/20' : confirmed ? 'bg-surface-elevated/50' : ''
    }`}>
      <div
        className="grid grid-cols-[40px_1fr_1fr_48px] sm:grid-cols-[40px_1fr_1fr_1fr_48px] gap-2 items-center py-2 px-2"
      >
        {/* Set number + type */}
        <div className="text-center">
          <button
            onClick={() => {
              if (confirmed) return
              const idx = setTypes.indexOf(setType)
              setSetType(setTypes[(idx + 1) % setTypes.length])
            }}
            className={`text-sm font-bold tap-target w-10 h-10 rounded-full flex items-center justify-center ${
              setType === 'warmup'
                ? 'text-warning bg-warning/10'
                : setType === 'drop_set'
                ? 'text-danger bg-danger/10'
                : setType === 'failure'
                ? 'text-danger bg-danger/20'
                : 'text-text-secondary'
            }`}
            title={setType}
            disabled={confirmed}
          >
            {setType === 'warmup' ? 'W' : setType === 'drop_set' ? 'D' : setType === 'failure' ? 'F' : setNumber}
          </button>
        </div>

        {/* Previous */}
        <div className="text-xs text-text-muted truncate text-center hidden sm:block">
          {previous || '-'}
        </div>

        {/* Weight */}
        <div className="flex items-center justify-center gap-1">
          <button
            onClick={() => setWeight(Math.max(0, weight - 2.5))}
            disabled={confirmed}
            className="tap-target w-8 h-8 flex items-center justify-center rounded-full bg-surface-elevated
                       text-text-secondary hover:text-text-primary active:scale-90 transition-all text-sm"
          >
            -
          </button>
          <input
            type="number"
            value={weight || ''}
            onChange={(e) => setWeight(Number(e.target.value) || 0)}
            placeholder="kg"
            disabled={confirmed}
            className="w-16 text-center text-xl font-mono font-bold bg-transparent text-text-primary
                       border-b-2 border-text-muted focus:border-primary transition-colors
                       [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none
                       [&::-webkit-inner-spin-button]:appearance-none"
          />
          <button
            onClick={() => setWeight(weight + 2.5)}
            disabled={confirmed}
            className="tap-target w-8 h-8 flex items-center justify-center rounded-full bg-surface-elevated
                       text-text-secondary hover:text-text-primary active:scale-90 transition-all text-sm"
          >
            +
          </button>
        </div>

        {/* Reps */}
        <div className="flex items-center justify-center gap-1">
          <button
            onClick={() => setReps(Math.max(0, reps - 1))}
            disabled={confirmed}
            className="tap-target w-8 h-8 flex items-center justify-center rounded-full bg-surface-elevated
                       text-text-secondary hover:text-text-primary active:scale-90 transition-all text-sm"
          >
            -
          </button>
          <input
            type="number"
            value={reps || ''}
            onChange={(e) => setReps(Number(e.target.value) || 0)}
            placeholder="reps"
            disabled={confirmed}
            className="w-12 text-center text-xl font-mono font-bold bg-transparent text-text-primary
                       border-b-2 border-text-muted focus:border-primary transition-colors
                       [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none
                       [&::-webkit-inner-spin-button]:appearance-none"
          />
          <button
            onClick={() => setReps(reps + 1)}
            disabled={confirmed}
            className="tap-target w-8 h-8 flex items-center justify-center rounded-full bg-surface-elevated
                       text-text-secondary hover:text-text-primary active:scale-90 transition-all text-sm"
          >
            +
          </button>
        </div>

        {/* Confirm */}
        <button
          onClick={handleConfirm}
          disabled={confirmed || (weight === 0 && reps === 0)}
          className={`tap-target w-12 h-12 flex items-center justify-center rounded-full transition-all active:scale-90 ${
            confirmed
              ? 'bg-success text-white'
              : weight > 0 || reps > 0
              ? 'bg-primary text-white hover:bg-primary/80'
              : 'bg-surface-elevated text-text-muted'
          }`}
        >
          <svg width="20" height="20" viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="2.5">
            <path d="M4 10l4 4 8-8" />
          </svg>
        </button>
      </div>

      {/* RPE selector */}
      {!confirmed && (
        <div className="flex items-center gap-1 px-2 pb-2 pt-1 overflow-x-auto no-scrollbar">
          <span className="text-xs text-text-muted mr-1 shrink-0">RPE</span>
          {rpeValues.map((val) => (
            <button
              key={val}
              onClick={() => setRpe(rpe === val ? undefined : val)}
              className={`shrink-0 px-2 py-1 rounded-lg text-xs font-medium transition-colors ${
                rpe === val
                  ? 'bg-primary text-white'
                  : 'bg-surface-elevated text-text-secondary hover:text-text-primary'
              }`}
            >
              {val}
            </button>
          ))}
        </div>
      )}
      {confirmed && rpe != null && (
        <div className="px-2 pb-2 pt-1">
          <span className="text-xs text-text-muted">RPE {rpe}</span>
        </div>
      )}
    </div>
  )
}

// --- PR Toast ---

interface PRToastProps {
  estimated1rm: number
  previous1rm?: number
  onDismiss: () => void
}

function PRToast({ estimated1rm, previous1rm, onDismiss }: PRToastProps) {
  useEffect(() => {
    const timer = setTimeout(onDismiss, 4000)
    return () => clearTimeout(timer)
  }, [onDismiss])

  return (
    <div className="fixed top-4 left-4 right-4 z-50 animate-bounce">
      <div className="bg-celebration/10 border-2 border-celebration rounded-card p-4 text-center shadow-lg">
        <div className="text-celebration text-lg font-bold">New PR!</div>
        <div className="text-text-primary font-mono text-sm mt-1">
          Est. 1RM: {estimated1rm.toFixed(1)}kg
          {previous1rm != null && (
            <span className="text-text-muted"> (prev: {previous1rm.toFixed(1)}kg)</span>
          )}
        </div>
      </div>
    </div>
  )
}

// --- Exercise Section ---

interface ExerciseSectionProps {
  exercise: Exercise
  sets: ExerciseSet[]
  history: ExerciseHistoryEntry[]
  workoutId: string
  onSetConfirmed: (isPR: boolean, estimated1rm?: number, previous1rm?: number) => void
}

function ExerciseSection({
  exercise,
  sets,
  history,
  workoutId,
  onSetConfirmed,
}: ExerciseSectionProps) {
  const { addSet, startRestTimer } = useWorkoutStore()
  const [inputRows, setInputRows] = useState(1)

  // Get last workout's sets for "Previous" display
  const lastSession = history[0]
  const previousSets = lastSession?.sets || []

  const previousDisplay = previousSets
    .filter((s) => s.set_type === 'working')
    .map((s) => `${s.weight_kg}kg x ${s.reps}`)
    .join(', ')

  const handleConfirmSet = async (data: {
    weight_kg: number
    reps: number
    rpe?: number
    set_type: ExerciseSet['set_type']
  }) => {
    // Optimistic: add to local store immediately
    const optimisticSet: ExerciseSet = {
      id: `temp-${Date.now()}`,
      workout_id: workoutId,
      exercise_id: exercise.id,
      set_order: sets.length + 1,
      exercise_order: 0,
      set_type: data.set_type,
      weight_kg: data.weight_kg,
      reps: data.reps,
      rpe: data.rpe,
      is_pr: false,
    }
    addSet(exercise.id, optimisticSet)

    // Start rest timer
    startRestTimer(getDefaultRest(data.set_type))

    // Add another input row
    setInputRows((r) => r + 1)

    // Sync with backend
    try {
      const response = await api.addSet(workoutId, {
        exercise_id: exercise.id,
        weight_kg: data.weight_kg,
        reps: data.reps,
        rpe: data.rpe,
        set_type: data.set_type,
      })
      if (response.is_pr) {
        onSetConfirmed(true, response.estimated_1rm, response.previous_1rm)
      } else {
        onSetConfirmed(false)
      }
    } catch {
      // Queue for retry offline
      useWorkoutStore.getState().addPendingSet(exercise.id, data)
    }
  }

  // Default weight/reps from previous session
  const getDefault = (setIndex: number) => {
    const prev = previousSets[setIndex]
    return {
      weight: prev?.weight_kg,
      reps: prev?.reps,
    }
  }

  const totalRows = Math.max(inputRows, previousSets.length, 1)

  return (
    <div className="card mb-4">
      {/* Exercise header */}
      <div className="flex items-center justify-between mb-3">
        <div>
          <h3 className="text-primary font-bold">{exercise.name}</h3>
          <span className="text-xs text-text-muted capitalize px-2 py-0.5 bg-surface-elevated rounded-full">
            {exercise.muscle_group}
          </span>
        </div>
      </div>

      {/* Previous performance */}
      {previousDisplay && (
        <div className="text-xs text-text-muted mb-3 px-2">
          Last time: {previousDisplay}
        </div>
      )}

      {/* Column headers */}
      <div className="grid grid-cols-[40px_1fr_1fr_48px] sm:grid-cols-[40px_1fr_1fr_1fr_48px] gap-2 px-2 mb-1">
        <div className="text-xs text-text-muted text-center">Set</div>
        <div className="text-xs text-text-muted text-center hidden sm:block">Previous</div>
        <div className="text-xs text-text-muted text-center">kg</div>
        <div className="text-xs text-text-muted text-center">Reps</div>
        <div className="text-xs text-text-muted text-center"></div>
      </div>

      {/* Set rows */}
      {Array.from({ length: totalRows }).map((_, i) => {
        const confirmedSet = sets[i]
        if (confirmedSet) {
          // Already confirmed row
          return (
            <div
              key={confirmedSet.id}
              className="grid grid-cols-[40px_1fr_1fr_48px] sm:grid-cols-[40px_1fr_1fr_1fr_48px] gap-2 items-center py-2 px-2 bg-surface-elevated/50 rounded-lg"
            >
              <div className="text-sm font-bold text-center text-text-secondary">{i + 1}</div>
              <div className="text-xs text-text-muted text-center hidden sm:block">
                {previousSets[i]
                  ? `${previousSets[i].weight_kg}x${previousSets[i].reps}`
                  : '-'}
              </div>
              <div className="text-xl font-mono font-bold text-center text-text-primary">
                {confirmedSet.weight_kg}
              </div>
              <div className="text-xl font-mono font-bold text-center text-text-primary">
                {confirmedSet.reps}
              </div>
              <div className="flex items-center justify-center">
                <div className="w-12 h-12 flex items-center justify-center rounded-full bg-success text-white">
                  <svg width="20" height="20" viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="2.5">
                    <path d="M4 10l4 4 8-8" />
                  </svg>
                </div>
              </div>
            </div>
          )
        }
        // Input row
        const defaults = getDefault(i)
        return (
          <SetInputRow
            key={`input-${i}`}
            setNumber={i + 1}
            previous={
              previousSets[i]
                ? `${previousSets[i].weight_kg}x${previousSets[i].reps}`
                : undefined
            }
            onConfirm={handleConfirmSet}
            defaultWeight={defaults.weight}
            defaultReps={defaults.reps}
          />
        )
      })}

      {/* Add set button */}
      <button
        onClick={() => setInputRows((r) => r + 1)}
        className="w-full mt-2 py-2 text-sm text-primary hover:text-primary/80 transition-colors"
      >
        + Add Set
      </button>
    </div>
  )
}

// --- Main ActiveWorkout Page ---

export default function ActiveWorkout() {
  const navigate = useNavigate()
  const {
    activeWorkoutId,
    workoutName,
    setWorkoutName,
    exercises,
    addExercise,
    removeExercise,
    elapsedSeconds,
    tickElapsed,
    currentExerciseIndex,
    setCurrentExerciseIndex,
    finishWorkout,
  } = useWorkoutStore()

  const [showPicker, setShowPicker] = useState(false)
  const [editingName, setEditingName] = useState(false)
  const [showFinishConfirm, setShowFinishConfirm] = useState(false)
  const [prToast, setPrToast] = useState<{
    estimated1rm: number
    previous1rm?: number
  } | null>(null)
  const [exerciseHistory, setExerciseHistory] = useState<Record<string, ExerciseHistoryEntry[]>>({})

  const elapsedRef = useRef<ReturnType<typeof setInterval> | null>(null)
  const nameInputRef = useRef<HTMLInputElement>(null)

  // Redirect if no active workout
  useEffect(() => {
    if (!activeWorkoutId) {
      navigate('/', { replace: true })
    }
  }, [activeWorkoutId, navigate])

  // Elapsed timer
  useEffect(() => {
    elapsedRef.current = setInterval(tickElapsed, 1000)
    return () => {
      if (elapsedRef.current) clearInterval(elapsedRef.current)
    }
  }, [tickElapsed])

  // Fetch history when exercise is added
  const fetchHistory = useCallback(async (exerciseId: string) => {
    try {
      const history = await api.getExerciseHistory(exerciseId)
      setExerciseHistory((prev) => ({ ...prev, [exerciseId]: history }))
    } catch {
      // History unavailable, proceed without it
    }
  }, [])

  const handleAddExercise = (exercise: Exercise) => {
    addExercise(exercise)
    setShowPicker(false)
    fetchHistory(exercise.id)
  }

  const handleFinish = async () => {
    if (!activeWorkoutId) return
    try {
      await api.completeWorkout(activeWorkoutId)
    } catch {
      // Workout may already be completed or network error
    }
    finishWorkout()
    navigate('/', { replace: true })
  }

  const handleSetConfirmed = (isPR: boolean, estimated1rm?: number, previous1rm?: number) => {
    if (isPR && estimated1rm != null) {
      setPrToast({ estimated1rm, previous1rm })
    }
  }

  const minutes = Math.floor(elapsedSeconds / 60)
  const seconds = elapsedSeconds % 60
  const elapsed = `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`

  // Compute total volume across all confirmed sets
  const totalVolume = exercises.reduce((sum, ex) => {
    return sum + ex.sets.reduce((setSum, s) => setSum + (s.weight_kg ?? 0) * (s.reps ?? 0), 0)
  }, 0)
  const formattedVolume = totalVolume.toLocaleString('en-US', { maximumFractionDigits: 0 })

  if (!activeWorkoutId) return null

  return (
    <div className="min-h-screen pb-32 bg-background">
      {/* Rest Timer overlay */}
      <RestTimer />

      {/* PR Toast */}
      {prToast && (
        <PRToast
          estimated1rm={prToast.estimated1rm}
          previous1rm={prToast.previous1rm}
          onDismiss={() => setPrToast(null)}
        />
      )}

      {/* Finish Confirmation Dialog */}
      {showFinishConfirm && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60">
          <div className="bg-surface rounded-card p-6 mx-4 max-w-sm w-full shadow-lg">
            <h2 className="text-lg font-bold text-text-primary mb-2">Finish workout?</h2>
            <p className="text-sm text-text-secondary mb-6">
              This will save your workout and end the session.
            </p>
            <div className="flex gap-3">
              <button
                onClick={() => setShowFinishConfirm(false)}
                className="flex-1 py-3 rounded-lg text-sm font-medium bg-surface-elevated text-text-secondary
                           hover:text-text-primary transition-colors"
              >
                Cancel
              </button>
              <button
                onClick={() => {
                  setShowFinishConfirm(false)
                  handleFinish()
                }}
                className="flex-1 py-3 rounded-lg text-sm font-medium btn-primary"
              >
                Finish
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Exercise Picker */}
      <ExercisePicker
        open={showPicker}
        onSelect={handleAddExercise}
        onClose={() => setShowPicker(false)}
      />

      {/* Header */}
      <div className="sticky top-0 z-30 bg-surface border-b border-text-muted">
        <div className="max-w-2xl mx-auto px-4 py-3 flex items-center justify-between">
          <div className="flex items-center gap-3 flex-1 min-w-0">
            {editingName ? (
              <input
                ref={nameInputRef}
                type="text"
                value={workoutName}
                onChange={(e) => setWorkoutName(e.target.value)}
                onBlur={() => setEditingName(false)}
                onKeyDown={(e) => {
                  if (e.key === 'Enter') setEditingName(false)
                }}
                className="text-lg font-bold bg-transparent border-b-2 border-primary text-text-primary
                           w-full max-w-[200px] outline-none"
                autoFocus
              />
            ) : (
              <button
                onClick={() => {
                  setEditingName(true)
                  setTimeout(() => nameInputRef.current?.focus(), 50)
                }}
                className="text-lg font-bold text-text-primary truncate hover:text-primary transition-colors"
              >
                {workoutName}
              </button>
            )}
            <div className="text-text-secondary font-mono text-sm bg-surface-elevated px-2 py-1 rounded shrink-0">
              {elapsed}
            </div>
            {totalVolume > 0 && (
              <div className="text-text-secondary font-mono text-xs bg-surface-elevated px-2 py-1 rounded shrink-0">
                {formattedVolume} kg
              </div>
            )}
          </div>
          <button
            onClick={() => setShowFinishConfirm(true)}
            className="btn-primary text-sm px-4 py-2 ml-3"
          >
            Finish
          </button>
        </div>
      </div>

      {/* Exercise tabs */}
      {exercises.length > 1 && (
        <div className="bg-surface border-b border-text-muted/30 overflow-x-auto no-scrollbar">
          <div className="max-w-2xl mx-auto px-4 flex gap-1 py-2">
            {exercises.map((ex, i) => (
              <button
                key={ex.exercise.id}
                onClick={() => setCurrentExerciseIndex(i)}
                className={`tap-target px-3 py-2 rounded-lg text-sm font-medium whitespace-nowrap transition-all ${
                  i === currentExerciseIndex
                    ? 'bg-primary text-white'
                    : 'text-text-secondary hover:text-text-primary'
                }`}
              >
                {ex.exercise.name}
              </button>
            ))}
          </div>
        </div>
      )}

      {/* Current exercise */}
      <div className="max-w-2xl mx-auto px-4 py-4">
        {exercises.length === 0 ? (
          <div className="text-center py-16">
            <div className="text-text-muted text-lg mb-2">No exercises yet</div>
            <div className="text-text-muted text-sm mb-6">
              Add an exercise to start logging sets
            </div>
          </div>
        ) : (
          exercises.map((ex, i) => (
            <div key={ex.exercise.id} className={i === currentExerciseIndex ? '' : 'hidden'}>
              <ExerciseSection
                exercise={ex.exercise}
                sets={ex.sets}
                history={exerciseHistory[ex.exercise.id] || []}
                workoutId={activeWorkoutId}
                onSetConfirmed={handleSetConfirmed}
              />
              <button
                onClick={() => removeExercise(i)}
                className="text-sm text-danger/60 hover:text-danger transition-colors mt-1"
              >
                Remove exercise
              </button>
            </div>
          ))
        )}
      </div>

      {/* Sticky bottom: Add Exercise */}
      <div className="fixed bottom-0 left-0 right-0 z-20 bg-surface border-t border-text-muted/30 p-4">
        <div className="max-w-2xl mx-auto">
          <button
            onClick={() => setShowPicker(true)}
            className="btn-primary w-full text-base py-4"
          >
            + Add Exercise
          </button>
        </div>
      </div>
    </div>
  )
}
