import { useEffect, useRef } from 'react'
import { useWorkoutStore } from '@/stores/workoutStore'

export default function RestTimer() {
  const {
    restTimerSeconds,
    restTimerTotal,
    restTimerRunning,
    tickRestTimer,
    adjustRestTimer,
    stopRestTimer,
  } = useWorkoutStore()

  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null)
  const hasVibratedRef = useRef(false)

  useEffect(() => {
    if (restTimerRunning) {
      hasVibratedRef.current = false
      intervalRef.current = setInterval(() => {
        tickRestTimer()
      }, 1000)
    }
    return () => {
      if (intervalRef.current) clearInterval(intervalRef.current)
    }
  }, [restTimerRunning, tickRestTimer])

  // Vibrate when timer completes
  useEffect(() => {
    if (!restTimerRunning && restTimerTotal > 0 && restTimerSeconds === 0 && !hasVibratedRef.current) {
      hasVibratedRef.current = true
      if (navigator.vibrate) {
        navigator.vibrate([200, 100, 200, 100, 200])
      }
    }
  }, [restTimerRunning, restTimerSeconds, restTimerTotal])

  if (!restTimerRunning && restTimerSeconds === 0) return null

  const progress = restTimerTotal > 0 ? restTimerSeconds / restTimerTotal : 0
  const radius = 54
  const circumference = 2 * Math.PI * radius
  const strokeDashoffset = circumference * (1 - progress)

  const minutes = Math.floor(restTimerSeconds / 60)
  const seconds = restTimerSeconds % 60
  const timeDisplay = `${minutes}:${seconds.toString().padStart(2, '0')}`

  const isFinished = !restTimerRunning && restTimerSeconds === 0 && restTimerTotal > 0

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/70 backdrop-blur-sm"
      onClick={stopRestTimer}
    >
      <div
        className="flex flex-col items-center gap-6"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="text-text-secondary text-sm font-medium uppercase tracking-wider">
          {isFinished ? 'Rest Complete' : 'Rest Timer'}
        </div>

        {/* Circular timer */}
        <div className="relative w-48 h-48">
          <svg className="w-full h-full transform -rotate-90" viewBox="0 0 120 120">
            {/* Background circle */}
            <circle
              cx="60"
              cy="60"
              r={radius}
              stroke="#1E1E2E"
              strokeWidth="8"
              fill="none"
            />
            {/* Progress circle */}
            <circle
              cx="60"
              cy="60"
              r={radius}
              stroke={isFinished ? '#00D68F' : '#6C5CE7'}
              strokeWidth="8"
              fill="none"
              strokeDasharray={circumference}
              strokeDashoffset={strokeDashoffset}
              strokeLinecap="round"
              className="transition-all duration-1000 ease-linear"
            />
          </svg>
          <div className="absolute inset-0 flex items-center justify-center">
            <span className="text-5xl font-mono font-bold text-text-primary">
              {isFinished ? 'GO' : timeDisplay}
            </span>
          </div>
        </div>

        {/* Adjust buttons */}
        {restTimerRunning && (
          <div className="flex gap-4">
            <button
              onClick={() => adjustRestTimer(-30)}
              className="tap-target px-5 py-3 bg-surface-elevated rounded-button text-text-secondary
                         text-lg font-mono hover:bg-surface active:scale-95 transition-all"
            >
              -30s
            </button>
            <button
              onClick={() => adjustRestTimer(30)}
              className="tap-target px-5 py-3 bg-surface-elevated rounded-button text-text-secondary
                         text-lg font-mono hover:bg-surface active:scale-95 transition-all"
            >
              +30s
            </button>
          </div>
        )}

        {/* Dismiss hint */}
        <button
          onClick={stopRestTimer}
          className="tap-target text-text-muted text-sm hover:text-text-secondary transition-colors"
        >
          Tap to dismiss
        </button>
      </div>
    </div>
  )
}
