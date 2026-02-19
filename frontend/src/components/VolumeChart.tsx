import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts'

interface VolumeData {
  muscle_group: string
  total_volume: number
  exercise_count: number
  total_reps: number
}

interface VolumeChartProps {
  data: VolumeData[]
  timeframe: 'week' | 'month'
}

export function VolumeChart({ data, timeframe }: VolumeChartProps) {
  // Convert muscle group names to friendly labels
  const friendlyLabels: Record<string, string> = {
    chest: 'Chest',
    back: 'Back',
    legs: 'Legs',
    shoulders: 'Shoulders',
    arms: 'Arms',
    core: 'Core',
  }

  const chartData = data.map((item) => ({
    name: friendlyLabels[item.muscle_group] || item.muscle_group,
    volume: Math.round(item.total_volume / 1000), // Convert to tons
    sets: item.exercise_count * 3, // Approximate sets
  }))

  return (
    <div className="bg-surface border border-border rounded-lg p-6">
      <h3 className="text-xl font-bold text-text-primary mb-4">
        Volume by Muscle Group (This {timeframe === 'week' ? 'Week' : 'Month'})
      </h3>
      <ResponsiveContainer width="100%" height={300}>
        <BarChart data={chartData}>
          <CartesianGrid strokeDasharray="3 3" stroke="#2C3440" />
          <XAxis dataKey="name" stroke="#B0B8C1" />
          <YAxis stroke="#B0B8C1" label={{ value: 'Tons', angle: -90, position: 'insideLeft' }} />
          <Tooltip
            contentStyle={{
              backgroundColor: '#1A1F28',
              border: '1px solid #2C3440',
              borderRadius: '8px',
            }}
            labelStyle={{ color: '#E8EAED' }}
          />
          <Legend />
          <Bar dataKey="volume" fill="#6366F1" name="Volume (tons)" />
        </BarChart>
      </ResponsiveContainer>
    </div>
  )
}
