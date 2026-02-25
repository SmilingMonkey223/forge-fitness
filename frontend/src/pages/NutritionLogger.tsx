import { useState, useMemo } from 'react'
import { useQuery, useQueryClient } from '@tanstack/react-query'
import { api } from '@/services/api'
import type { NutritionLog } from '@/types'
import FoodSearch from '@/components/FoodSearch'

function formatDate(date: Date): string {
  return date.toISOString().split('T')[0]
}

function displayDate(dateStr: string): string {
  const d = new Date(dateStr + 'T00:00:00')
  const today = new Date()
  const yesterday = new Date(today)
  yesterday.setDate(yesterday.getDate() - 1)

  if (dateStr === formatDate(today)) return 'Today'
  if (dateStr === formatDate(yesterday)) return 'Yesterday'
  return d.toLocaleDateString('en-US', { weekday: 'short', month: 'short', day: 'numeric' })
}

type MealType = 'breakfast' | 'lunch' | 'dinner' | 'snack'

const MEAL_ORDER: MealType[] = ['breakfast', 'lunch', 'dinner', 'snack']
const MEAL_LABELS: Record<MealType, string> = {
  breakfast: 'Breakfast',
  lunch: 'Lunch',
  dinner: 'Dinner',
  snack: 'Snacks',
}

function MacroBar({ consumed, target, color }: { consumed: number; target: number; color: string }) {
  const pct = target > 0 ? Math.min((consumed / target) * 100, 100) : 0
  return (
    <div className="flex-1">
      <div className="h-2 bg-surface-elevated rounded-full overflow-hidden">
        <div className="h-full rounded-full transition-all duration-500" style={{ width: `${pct}%`, backgroundColor: color }} />
      </div>
    </div>
  )
}

export function NutritionLogger() {
  const queryClient = useQueryClient()
  const [date, setDate] = useState(() => formatDate(new Date()))
  const [expandedMeals, setExpandedMeals] = useState<Set<MealType>>(new Set(MEAL_ORDER))
  const [foodSearchOpen, setFoodSearchOpen] = useState(false)
  const [foodSearchMeal, setFoodSearchMeal] = useState<string>('breakfast')
  const [deletingId, setDeletingId] = useState<string | null>(null)

  const { data: logs, isLoading } = useQuery({
    queryKey: ['nutrition-logs', date],
    queryFn: () => api.getNutritionLog(date),
  })

  const { data: summary } = useQuery({
    queryKey: ['nutrition-summary', date],
    queryFn: () => api.getNutritionSummary(date),
  })

  const mealGroups = useMemo(() => {
    const groups: Record<MealType, NutritionLog[]> = {
      breakfast: [],
      lunch: [],
      dinner: [],
      snack: [],
    }
    for (const log of logs || []) {
      const meal = (log.meal_type || 'snack') as MealType
      if (groups[meal]) groups[meal].push(log)
      else groups.snack.push(log)
    }
    return groups
  }, [logs])

  const changeDate = (delta: number) => {
    const d = new Date(date + 'T00:00:00')
    d.setDate(d.getDate() + delta)
    setDate(formatDate(d))
  }

  const toggleMeal = (meal: MealType) => {
    setExpandedMeals((prev) => {
      const next = new Set(prev)
      if (next.has(meal)) next.delete(meal)
      else next.add(meal)
      return next
    })
  }

  const openFoodSearch = (meal: string) => {
    setFoodSearchMeal(meal)
    setFoodSearchOpen(true)
  }

  const handleDelete = async (id: string) => {
    setDeletingId(id)
    try {
      await api.deleteNutritionLog(id)
      queryClient.invalidateQueries({ queryKey: ['nutrition-logs', date] })
      queryClient.invalidateQueries({ queryKey: ['nutrition-summary', date] })
    } catch {
      alert('Failed to delete entry')
    } finally {
      setDeletingId(null)
    }
  }

  const handleFoodLogged = () => {
    queryClient.invalidateQueries({ queryKey: ['nutrition-logs', date] })
    queryClient.invalidateQueries({ queryKey: ['nutrition-summary', date] })
    queryClient.invalidateQueries({ queryKey: ['dashboard'] })
  }

  const totalCals = summary?.total_calories || 0
  const totalProtein = summary?.total_protein_g || 0
  const totalCarbs = summary?.total_carbs_g || 0
  const totalFat = summary?.total_fat_g || 0
  // Targets would come from profile; using defaults
  const targetCals = 2400
  const targetProtein = 180
  const targetCarbs = 280
  const targetFat = 80

  return (
    <div className="min-h-screen bg-background pb-20">
      <FoodSearch
        open={foodSearchOpen}
        mealType={foodSearchMeal}
        onClose={() => setFoodSearchOpen(false)}
        onLogged={handleFoodLogged}
      />

      {/* Header */}
      <div className="bg-surface border-b border-text-muted">
        <div className="max-w-2xl mx-auto px-4 py-4">
          <h1 className="text-2xl font-bold text-center mb-4">Nutrition</h1>
          {/* Date selector */}
          <div className="flex items-center justify-center gap-4">
            <button onClick={() => changeDate(-1)} className="tap-target p-2 text-text-secondary hover:text-text-primary">
              <svg width="20" height="20" viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M12 4l-6 6 6 6" />
              </svg>
            </button>
            <button
              onClick={() => {
                const input = document.createElement('input')
                input.type = 'date'
                input.value = date
                input.onchange = () => setDate(input.value)
                input.click()
              }}
              className="text-lg font-medium text-text-primary hover:text-primary transition-colors"
            >
              {displayDate(date)}
            </button>
            <button onClick={() => changeDate(1)} className="tap-target p-2 text-text-secondary hover:text-text-primary">
              <svg width="20" height="20" viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M8 4l6 6-6 6" />
              </svg>
            </button>
          </div>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-4 space-y-4">
        {/* Daily summary */}
        <div className="card">
          <div className="flex items-center justify-between mb-3">
            <div>
              <div className="text-2xl font-bold font-mono text-primary">{totalCals}</div>
              <div className="text-xs text-text-muted">/ {targetCals} kcal</div>
            </div>
            <div className="flex gap-6 text-center">
              <div>
                <div className="text-sm font-mono font-bold text-success">{Math.round(totalProtein)}g</div>
                <div className="text-xs text-text-muted">Protein</div>
              </div>
              <div>
                <div className="text-sm font-mono font-bold text-warning">{Math.round(totalCarbs)}g</div>
                <div className="text-xs text-text-muted">Carbs</div>
              </div>
              <div>
                <div className="text-sm font-mono font-bold text-danger">{Math.round(totalFat)}g</div>
                <div className="text-xs text-text-muted">Fat</div>
              </div>
            </div>
          </div>
          <div className="flex gap-2">
            <MacroBar consumed={totalProtein} target={targetProtein} color="#00D68F" />
            <MacroBar consumed={totalCarbs} target={targetCarbs} color="#FFB800" />
            <MacroBar consumed={totalFat} target={targetFat} color="#FF5252" />
          </div>
        </div>

        {/* Loading */}
        {isLoading ? (
          <div className="text-center py-12 text-text-muted">Loading meals...</div>
        ) : (
          /* Meal groups */
          MEAL_ORDER.map((meal) => {
            const entries = mealGroups[meal]
            const mealCals = entries.reduce((s, e) => s + e.calories, 0)
            const isExpanded = expandedMeals.has(meal)

            return (
              <div key={meal} className="card p-0 overflow-hidden">
                {/* Meal header */}
                <button
                  onClick={() => toggleMeal(meal)}
                  className="tap-target w-full flex items-center justify-between px-4 py-3 hover:bg-surface-elevated/50 transition-colors"
                >
                  <div className="flex items-center gap-3">
                    <svg
                      width="16" height="16" viewBox="0 0 16 16" fill="currentColor"
                      className={`text-text-muted transition-transform ${isExpanded ? 'rotate-90' : ''}`}
                    >
                      <path d="M6 3l5 5-5 5V3z" />
                    </svg>
                    <span className="font-medium text-text-primary">{MEAL_LABELS[meal]}</span>
                    <span className="text-xs text-text-muted">{entries.length} items</span>
                  </div>
                  <span className="font-mono text-sm text-text-secondary">{mealCals} cal</span>
                </button>

                {isExpanded && (
                  <div className="border-t border-surface-elevated">
                    {entries.length === 0 ? (
                      <div className="px-4 py-6 text-center text-text-muted text-sm">
                        No food logged yet
                      </div>
                    ) : (
                      entries.map((entry) => (
                        <div key={entry.id} className="flex items-center justify-between px-4 py-3 border-b border-surface-elevated/50 last:border-b-0">
                          <div className="min-w-0 flex-1">
                            <div className="text-sm font-medium text-text-primary truncate">{entry.food_name}</div>
                            <div className="text-xs text-text-muted">
                              {entry.serving_size} {entry.serving_unit} x {entry.quantity} — P:{Math.round(entry.protein_g)}g C:{Math.round(entry.carbs_g)}g F:{Math.round(entry.fat_g)}g
                            </div>
                          </div>
                          <div className="flex items-center gap-2 ml-2">
                            <span className="font-mono text-sm text-primary">{entry.calories}</span>
                            <button
                              onClick={() => handleDelete(entry.id)}
                              disabled={deletingId === entry.id}
                              className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-danger transition-colors"
                            >
                              <svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
                                <path d="M4 4l8 8M12 4l-8 8" />
                              </svg>
                            </button>
                          </div>
                        </div>
                      ))
                    )}
                    {/* Add food button */}
                    <button
                      onClick={() => openFoodSearch(meal)}
                      className="tap-target w-full py-3 text-sm text-primary hover:bg-surface-elevated/30 transition-colors"
                    >
                      + Add Food
                    </button>
                  </div>
                )}
              </div>
            )
          })
        )}
      </div>
    </div>
  )
}
