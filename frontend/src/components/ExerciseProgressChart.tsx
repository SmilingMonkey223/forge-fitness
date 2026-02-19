import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts'

interface ProgressDataPoint {
  date: string
  estimated_1rm: number
  weight_lbs: number
  reps: number
}

interface ExerciseProgressChartProps {
  exerciseName: string
  data: ProgressDataPoint[]
}

export function ExerciseProgressChart({ exerciseName, data }: ExerciseProgressChartProps) {
  const chartData = data.map((point) => ({
    date: new Date(point.date).toLocaleDateString('en-US', { month: 'short', day: 'numeric' }),
    e1rm: Math.round(point.estimated_1rm),
  }))

  return (
    <div className="bg-surface border border-border rounded-lg p-6">
      <h3 className="text-xl font-bold text-text-primary mb-4">
        {exerciseName} Progress
      </h3>
      <div className="text-sm text-text-secondary mb-4">
        Estimated 1RM over time
      </div>

      {data.length === 0 ? (
        <div className="text-center py-12 text-text-secondary">
          No data yet for this exercise
        </div>
      ) : (
        <ResponsiveContainer width="100%" height={250}>
          <LineChart data={chartData}>
            <CartesianGrid strokeDasharray="3 3" stroke="#2C3440" />
            <XAxis dataKey="date" stroke="#B0B8C1" />
            <YAxis stroke="#B0B8C1" label={{ value: 'lbs', angle: -90, position: 'insideLeft' }} />
            <Tooltip
              contentStyle={{
                backgroundColor: '#1A1F28',
                border: '1px solid #2C3440',
                borderRadius: '8px',
              }}
              labelStyle={{ color: '#E8EAED' }}
            />
            <Line
              type="monotone"
              dataKey="e1rm"
              stroke="#10B981"
              strokeWidth={3}
              dot={{ fill: '#10B981', r: 4 }}
              activeDot={{ r: 6 }}
              name="Estimated 1RM"
            />
          </LineChart>
        </ResponsiveContainer>
      )}

      {data.length > 0 && (
        <div className="mt-4 grid grid-cols-2 gap-4">
          <div className="bg-background p-3 rounded-lg border border-border">
            <div className="text-xs text-text-secondary mb-1">Current</div>
            <div className="text-2xl font-bold text-primary">
              {Math.round(data[data.length - 1].estimated_1rm)} lbs
            </div>
          </div>
          <div className="bg-background p-3 rounded-lg border border-border">
            <div className="text-xs text-text-secondary mb-1">Progress</div>
            <div className="text-2xl font-bold text-success">
              +{Math.round(data[data.length - 1].estimated_1rm - data[0].estimated_1rm)} lbs
            </div>
          </div>
        </div>
      )}
    </div>
  )
}
