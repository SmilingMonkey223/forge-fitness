import React, { useState } from 'react';
import { ExerciseVideoPlayer } from '../components/ExerciseVideoPlayer';

interface Exercise {
  id: string;
  name: string;
  sets: number;
  reps: string; // e.g., "8-12"
  targetRIR: number;
}

interface LoggedSet {
  setNumber: number;
  weight: number;
  reps: number;
  rir: number;
  partialReps: boolean;
  toFailure: boolean;
}

export const WorkoutLogger: React.FC = () => {
  const [exercises] = useState<Exercise[]>([
    {
      id: '1',
      name: 'Bench Press',
      sets: 3,
      reps: '8-12',
      targetRIR: 2,
    },
    {
      id: '2',
      name: 'Incline Dumbbell Press',
      sets: 3,
      reps: '10-15',
      targetRIR: 3,
    },
    {
      id: '3',
      name: 'Cable Flyes',
      sets: 3,
      reps: '12-15',
      targetRIR: 2,
    },
  ]);

  const [currentExerciseIndex, setCurrentExerciseIndex] = useState(0);
  const [loggedSets, setLoggedSets] = useState<Map<string, LoggedSet[]>>(new Map());
  const [showVideo, setShowVideo] = useState(false);

  // Current set logging state
  const [weight, setWeight] = useState(135);
  const [reps, setReps] = useState(10);
  const [rir, setRir] = useState(2);
  const [partialReps, setPartialReps] = useState(false);
  const [toFailure, setToFailure] = useState(false);

  // Rest timer
  const [restTimeRemaining, setRestTimeRemaining] = useState<number | null>(null);

  const currentExercise = exercises[currentExerciseIndex];
  const currentSets = loggedSets.get(currentExercise.id) || [];
  const currentSetNumber = currentSets.length + 1;

  const logSet = () => {
    const newSet: LoggedSet = {
      setNumber: currentSetNumber,
      weight,
      reps,
      rir,
      partialReps,
      toFailure,
    };

    const updatedSets = [...currentSets, newSet];
    setLoggedSets(new Map(loggedSets.set(currentExercise.id, updatedSets)));

    // Start rest timer (90 seconds)
    setRestTimeRemaining(90);
    const interval = setInterval(() => {
      setRestTimeRemaining(prev => {
        if (prev === null || prev <= 1) {
          clearInterval(interval);
          return null;
        }
        return prev - 1;
      });
    }, 1000);

    // Reset toggles
    setPartialReps(false);
    setToFailure(false);
  };

  const skipSet = () => {
    // Move to next set/exercise without logging
    if (currentSetNumber >= currentExercise.sets) {
      nextExercise();
    }
  };

  const nextExercise = () => {
    if (currentExerciseIndex < exercises.length - 1) {
      setCurrentExerciseIndex(currentExerciseIndex + 1);
    }
  };

  const previousExercise = () => {
    if (currentExerciseIndex > 0) {
      setCurrentExerciseIndex(currentExerciseIndex - 1);
    }
  };

  const completeWorkout = () => {
    // TODO: Save workout to backend
    alert('Workout completed! 🎉');
  };

  return (
    <div className="min-h-screen bg-gray-950 text-white p-4">
      {/* Header */}
      <div className="max-w-2xl mx-auto">
        <div className="flex justify-between items-center mb-6">
          <h1 className="text-2xl font-bold">Push Day A</h1>
          <button
            onClick={completeWorkout}
            className="px-4 py-2 bg-green-600 hover:bg-green-700 rounded-lg font-semibold"
          >
            Finish Workout
          </button>
        </div>

        {/* Exercise Progress */}
        <div className="mb-6">
          <div className="flex justify-between text-sm text-gray-400 mb-2">
            <span>Exercise {currentExerciseIndex + 1} of {exercises.length}</span>
            <span>Set {currentSetNumber} of {currentExercise.sets}</span>
          </div>
          <div className="w-full bg-gray-800 rounded-full h-2">
            <div
              className="bg-blue-600 h-2 rounded-full transition-all"
              style={{
                width: `${((currentExerciseIndex + (currentSetNumber / currentExercise.sets)) / exercises.length) * 100}%`
              }}
            ></div>
          </div>
        </div>

        {/* Current Exercise */}
        <div className="bg-gray-900 rounded-xl p-6 mb-6">
          <div className="flex justify-between items-start mb-4">
            <div>
              <h2 className="text-2xl font-bold mb-1">{currentExercise.name}</h2>
              <p className="text-gray-400">
                Target: {currentExercise.sets} sets × {currentExercise.reps} reps @ {currentExercise.targetRIR} RIR
              </p>
            </div>
            <button
              onClick={() => setShowVideo(!showVideo)}
              className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg text-sm font-semibold"
            >
              📹 Form Video
            </button>
          </div>

          {/* Video Player */}
          {showVideo && (
            <div className="mb-4">
              <ExerciseVideoPlayer
                exerciseId={currentExercise.id}
                exerciseName={currentExercise.name}
                isInWorkout={true}
                onClose={() => setShowVideo(false)}
              />
            </div>
          )}

          {/* Previous Sets */}
          {currentSets.length > 0 && (
            <div className="mb-4">
              <h3 className="text-sm font-semibold text-gray-400 mb-2">Completed Sets:</h3>
              <div className="space-y-2">
                {currentSets.map(set => (
                  <div key={set.setNumber} className="flex justify-between items-center bg-gray-800 rounded p-3">
                    <span className="text-sm text-gray-400">Set {set.setNumber}</span>
                    <span className="font-semibold">{set.weight} lbs × {set.reps}</span>
                    <span className="text-sm text-blue-400">{set.rir} RIR</span>
                    {set.toFailure && <span className="text-xs bg-red-600 px-2 py-1 rounded">Failure</span>}
                    {set.partialReps && <span className="text-xs bg-orange-600 px-2 py-1 rounded">Partial</span>}
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Rest Timer */}
          {restTimeRemaining !== null && (
            <div className="mb-4 p-4 bg-blue-900/50 rounded-lg text-center">
              <p className="text-sm text-gray-300 mb-1">Rest Time</p>
              <p className="text-4xl font-bold">{Math.floor(restTimeRemaining / 60)}:{(restTimeRemaining % 60).toString().padStart(2, '0')}</p>
              <button
                onClick={() => setRestTimeRemaining(null)}
                className="mt-2 text-sm text-blue-400 hover:text-blue-300"
              >
                Skip Rest
              </button>
            </div>
          )}

          {/* Set Logging */}
          <div className="space-y-4">
            <div>
              <label className="block text-sm font-medium text-gray-300 mb-2">Weight (lbs)</label>
              <div className="flex items-center gap-2">
                <button
                  onClick={() => setWeight(w => Math.max(0, w - 5))}
                  className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded font-bold"
                >
                  -5
                </button>
                <input
                  type="number"
                  value={weight}
                  onChange={e => setWeight(Number(e.target.value))}
                  className="flex-1 bg-gray-800 border border-gray-700 rounded px-4 py-2 text-center text-2xl font-bold"
                />
                <button
                  onClick={() => setWeight(w => w + 5)}
                  className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded font-bold"
                >
                  +5
                </button>
              </div>
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-300 mb-2">Reps</label>
              <div className="flex items-center gap-2">
                <button
                  onClick={() => setReps(r => Math.max(1, r - 1))}
                  className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded font-bold"
                >
                  -1
                </button>
                <input
                  type="number"
                  value={reps}
                  onChange={e => setReps(Number(e.target.value))}
                  className="flex-1 bg-gray-800 border border-gray-700 rounded px-4 py-2 text-center text-2xl font-bold"
                />
                <button
                  onClick={() => setReps(r => r + 1)}
                  className="px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded font-bold"
                >
                  +1
                </button>
              </div>
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-300 mb-2">
                RIR (Reps in Reserve) - Target: {currentExercise.targetRIR}
              </label>
              <div className="flex gap-1">
                {[0, 1, 2, 3, 4, 5].map(value => (
                  <button
                    key={value}
                    onClick={() => setRir(value)}
                    className={`flex-1 py-3 rounded font-bold transition-colors ${
                      rir === value
                        ? 'bg-blue-600 text-white'
                        : 'bg-gray-800 text-gray-400 hover:bg-gray-700'
                    }`}
                  >
                    {value}
                  </button>
                ))}
              </div>
              <p className="text-xs text-gray-500 mt-1">
                0 = Failure, 1 = Could do 1 more, 2 = Could do 2 more, etc.
              </p>
            </div>

            {/* Toggles */}
            <div className="flex gap-4">
              <label className="flex items-center gap-2 cursor-pointer">
                <input
                  type="checkbox"
                  checked={partialReps}
                  onChange={e => setPartialReps(e.target.checked)}
                  className="w-5 h-5"
                />
                <span className="text-sm">Partial Reps</span>
              </label>
              <label className="flex items-center gap-2 cursor-pointer">
                <input
                  type="checkbox"
                  checked={toFailure}
                  onChange={e => setToFailure(e.target.checked)}
                  className="w-5 h-5"
                />
                <span className="text-sm">Taken to Failure</span>
              </label>
            </div>

            {/* Action Buttons */}
            <div className="flex gap-2">
              <button
                onClick={logSet}
                className="flex-1 py-4 bg-blue-600 hover:bg-blue-700 rounded-lg font-bold text-lg"
              >
                ✓ Log Set {currentSetNumber}
              </button>
              <button
                onClick={skipSet}
                className="px-6 py-4 bg-gray-700 hover:bg-gray-600 rounded-lg font-semibold"
              >
                Skip
              </button>
            </div>

            {/* Exercise Navigation */}
            {currentSetNumber > currentExercise.sets && (
              <button
                onClick={nextExercise}
                className="w-full py-4 bg-green-600 hover:bg-green-700 rounded-lg font-bold text-lg"
              >
                Next Exercise →
              </button>
            )}
          </div>
        </div>

        {/* Exercise List */}
        <div className="bg-gray-900 rounded-xl p-4">
          <h3 className="font-semibold mb-3">Workout Exercises</h3>
          <div className="space-y-2">
            {exercises.map((ex, idx) => {
              const sets = loggedSets.get(ex.id) || [];
              const isComplete = sets.length >= ex.sets;
              const isCurrent = idx === currentExerciseIndex;

              return (
                <div
                  key={ex.id}
                  onClick={() => setCurrentExerciseIndex(idx)}
                  className={`p-3 rounded-lg cursor-pointer transition-colors ${
                    isCurrent
                      ? 'bg-blue-900/50 border border-blue-600'
                      : isComplete
                      ? 'bg-green-900/30 border border-green-700'
                      : 'bg-gray-800 hover:bg-gray-750'
                  }`}
                >
                  <div className="flex justify-between items-center">
                    <div>
                      <span className="font-medium">{ex.name}</span>
                      <span className="text-sm text-gray-400 ml-2">
                        {sets.length}/{ex.sets} sets
                      </span>
                    </div>
                    {isComplete && <span className="text-green-500">✓</span>}
                    {isCurrent && <span className="text-blue-400">▶</span>}
                  </div>
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </div>
  );
};
