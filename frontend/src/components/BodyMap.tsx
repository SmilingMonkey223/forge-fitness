interface MuscleGroupVolume {
  muscle_group: string
  total_volume: number
  total_reps: number
  percentage: number
}

interface BodyMapProps {
  volumeData: MuscleGroupVolume[]
}

export function BodyMap({ volumeData }: BodyMapProps) {
  // Calculate max volume for color intensity
  const maxVolume = Math.max(...volumeData.map((d) => d.total_volume))

  // Get color intensity based on volume (0-100%)
  const getColorIntensity = (volume: number) => {
    const intensity = (volume / maxVolume) * 100
    if (intensity > 75) return 'bg-red-500'
    if (intensity > 50) return 'bg-orange-500'
    if (intensity > 25) return 'bg-yellow-500'
    return 'bg-green-500'
  }

  const muscleGroups = [
    { name: 'chest', label: 'Chest', position: 'top-[25%] left-[50%] -translate-x-1/2' },
    { name: 'shoulders', label: 'Shoulders', position: 'top-[20%] left-[50%] -translate-x-1/2' },
    { name: 'back', label: 'Back', position: 'top-[30%] right-[10%]' },
    { name: 'arms', label: 'Arms', position: 'top-[35%] left-[15%]' },
    { name: 'core', label: 'Core', position: 'top-[45%] left-[50%] -translate-x-1/2' },
    { name: 'legs', label: 'Legs', position: 'top-[65%] left-[50%] -translate-x-1/2' },
  ]

  return (
    <div className="bg-surface border border-border rounded-lg p-6">
      <h3 className="text-xl font-bold text-text-primary mb-4">Training Distribution</h3>
      <p className="text-sm text-text-secondary mb-6">
        Heat map showing which muscle groups you've trained most
      </p>

      {/* Simple body visualization */}
      <div className="relative w-full h-96 bg-background rounded-lg mb-6">
        {/* Body outline (simplified) */}
        <svg className="absolute inset-0 w-full h-full" viewBox="0 0 200 300">
          {/* Head */}
          <circle cx="100" cy="30" r="20" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />

          {/* Torso */}
          <rect x="75" y="55" width="50" height="80" rx="10" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />

          {/* Arms */}
          <rect x="45" y="60" width="25" height="70" rx="8" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />
          <rect x="130" y="60" width="25" height="70" rx="8" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />

          {/* Legs */}
          <rect x="80" y="140" width="18" height="100" rx="8" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />
          <rect x="102" y="140" width="18" height="100" rx="8" fill="#2C3440" stroke="#B0B8C1" strokeWidth="2" />
        </svg>

        {/* Muscle group indicators */}
        {muscleGroups.map((group) => {
          const data = volumeData.find((d) => d.muscle_group === group.name)
          const volume = data?.total_volume || 0
          const reps = data?.total_reps || 0

          return (
            <div
              key={group.name}
              className={`absolute ${group.position} group cursor-pointer`}
            >
              <div
                className={`w-8 h-8 rounded-full border-2 border-white ${
                  volume > 0 ? getColorIntensity(volume) : 'bg-gray-600'
                } opacity-60 group-hover:opacity-100 transition-opacity`}
              />

              {/* Tooltip on hover */}
              <div className="absolute bottom-full left-1/2 -translate-x-1/2 mb-2 hidden group-hover:block bg-background border border-border rounded-lg p-2 whitespace-nowrap shadow-lg z-10">
                <div className="text-xs font-medium text-text-primary">{group.label}</div>
                <div className="text-xs text-text-secondary">
                  {(volume / 1000).toFixed(1)} tons • {reps} reps
                </div>
              </div>
            </div>
          )
        })}
      </div>

      {/* Legend */}
      <div className="space-y-2">
        <div className="text-sm font-medium text-text-secondary mb-2">Volume Intensity</div>
        <div className="flex items-center gap-2">
          <div className="w-6 h-6 bg-green-500 rounded"></div>
          <span className="text-xs text-text-secondary">Low</span>
          <div className="w-6 h-6 bg-yellow-500 rounded ml-2"></div>
          <span className="text-xs text-text-secondary">Medium</span>
          <div className="w-6 h-6 bg-orange-500 rounded ml-2"></div>
          <span className="text-xs text-text-secondary">High</span>
          <div className="w-6 h-6 bg-red-500 rounded ml-2"></div>
          <span className="text-xs text-text-secondary">Very High</span>
        </div>
      </div>
    </div>
  )
}
