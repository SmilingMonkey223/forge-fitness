interface PersonalRecord {
  exercise_name: string
  weight_lbs: number
  reps: number
  estimated_1rm: number
  achieved_at: string
}

interface PRListProps {
  records: PersonalRecord[]
}

export function PRList({ records }: PRListProps) {
  const formatDate = (dateString: string) => {
    const date = new Date(dateString)
    const now = new Date()
    const diffTime = Math.abs(now.getTime() - date.getTime())
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24))

    if (diffDays === 0) return 'Today'
    if (diffDays === 1) return 'Yesterday'
    if (diffDays < 7) return `${diffDays} days ago`
    if (diffDays < 30) return `${Math.floor(diffDays / 7)} weeks ago`
    return `${Math.floor(diffDays / 30)} months ago`
  }

  return (
    <div className="bg-surface border border-border rounded-lg p-6">
      <h3 className="text-xl font-bold text-text-primary mb-4">Recent Personal Records</h3>

      {records.length === 0 ? (
        <div className="text-center py-8 text-text-secondary">
          No PRs yet - keep training!
        </div>
      ) : (
        <div className="space-y-3">
          {records.map((pr, index) => (
            <div
              key={index}
              className="flex items-center justify-between p-4 bg-background rounded-lg border border-border hover:border-primary transition-colors"
            >
              <div className="flex-1">
                <div className="font-medium text-text-primary">{pr.exercise_name}</div>
                <div className="text-sm text-text-secondary">
                  {pr.weight_lbs} lbs × {pr.reps} reps
                  <span className="mx-2">•</span>
                  Est. 1RM: {Math.round(pr.estimated_1rm)} lbs
                </div>
              </div>
              <div className="text-right">
                <div className="text-2xl">🎉</div>
                <div className="text-xs text-text-secondary">{formatDate(pr.achieved_at)}</div>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}
