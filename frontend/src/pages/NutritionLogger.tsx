import { useState, useMemo } from 'react'
import { Link } from 'react-router-dom'
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
  const [errorMessage, setErrorMessage] = useState<string | null>(null)
  const [editingEntry, setEditingEntry] = useState<string | null>(null)
  const [editForm, setEditForm] = useState<{
    food_name: string; calories: number; protein_g: number; carbs_g: number; fat_g: number; quantity: number
  }>({ food_name: '', calories: 0, protein_g: 0, carbs_g: 0, fat_g: 0, quantity: 1 })
  const [editSaving, setEditSaving] = useState(false)

  const { data: logs, isLoading } = useQuery({
    queryKey: ['nutrition-logs', date],
    queryFn: () => api.getNutritionLog(date),
  })

  const { data: summary } = useQuery({
    queryKey: ['nutrition-summary', date],
    queryFn: () => api.getNutritionSummary(date),
  })

  const { data: profileData } = useQuery({
    queryKey: ['profile'],
    queryFn: () => api.getProfile(),
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
      setErrorMessage('Failed to delete entry. Please try again.')
    } finally {
      setDeletingId(null)
    }
  }

  const handleEditStart = (entry: NutritionLog) => {
    setEditingEntry(entry.id)
    setEditForm({
      food_name: entry.food_name,
      calories: entry.calories,
      protein_g: entry.protein_g,
      carbs_g: entry.carbs_g,
      fat_g: entry.fat_g,
      quantity: entry.quantity,
    })
    setErrorMessage(null)
  }

  const handleEditSave = async (id: string) => {
    setEditSaving(true)
    setErrorMessage(null)
    try {
      await api.updateNutritionLog(id, {
        food_name: editForm.food_name,
        calories: editForm.calories,
        protein_g: editForm.protein_g,
        carbs_g: editForm.carbs_g,
        fat_g: editForm.fat_g,
        quantity: editForm.quantity,
      })
      queryClient.invalidateQueries({ queryKey: ['nutrition-logs', date] })
      queryClient.invalidateQueries({ queryKey: ['nutrition-summary', date] })
      setEditingEntry(null)
    } catch {
      setErrorMessage('Failed to update entry. Please try again.')
    } finally {
      setEditSaving(false)
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

  const profile = profileData?.profile
  const hasTargets = !!(profile?.target_calories && profile?.target_protein_g && profile?.target_carbs_g && profile?.target_fat_g)
  const targetCals = profile?.target_calories || 0
  const targetProtein = profile?.target_protein_g || 0
  const targetCarbs = profile?.target_carbs_g || 0
  const targetFat = profile?.target_fat_g || 0

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
            <label className="relative cursor-pointer">
              <span className="text-lg font-medium text-text-primary hover:text-primary transition-colors">
                {displayDate(date)}
              </span>
              <input
                type="date"
                value={date}
                onChange={(e) => { if (e.target.value) setDate(e.target.value) }}
                className="absolute inset-0 opacity-0 cursor-pointer bg-surface-elevated border-border rounded-lg text-text-primary"
              />
            </label>
            <button onClick={() => changeDate(1)} className="tap-target p-2 text-text-secondary hover:text-text-primary">
              <svg width="20" height="20" viewBox="0 0 20 20" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M8 4l6 6-6 6" />
              </svg>
            </button>
          </div>
        </div>
      </div>

      <div className="max-w-2xl mx-auto px-4 py-4 space-y-4">
        {/* Error banner */}
        {errorMessage && (
          <div className="bg-danger/10 text-danger border border-danger/30 rounded-card px-4 py-3 flex items-center justify-between">
            <span className="text-sm">{errorMessage}</span>
            <button onClick={() => setErrorMessage(null)} className="ml-2 text-danger hover:text-danger/70">
              <svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
                <path d="M4 4l8 8M12 4l-8 8" />
              </svg>
            </button>
          </div>
        )}

        {/* Daily summary */}
        {!hasTargets ? (
          <div className="card text-center py-6">
            <div className="text-2xl font-bold font-mono text-primary mb-1">{totalCals} kcal</div>
            <div className="text-sm text-text-muted mb-3">
              Complete your profile to see personalized macro targets and progress bars.
            </div>
            <Link
              to="/settings"
              className="inline-block text-sm font-medium text-primary hover:text-primary/80 underline underline-offset-2"
            >
              Go to Profile Settings
            </Link>
          </div>
        ) : (
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
        )}

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
                      entries.map((entry) =>
                        editingEntry === entry.id ? (
                          /* Inline edit form */
                          <div key={entry.id} className="px-4 py-3 border-b border-surface-elevated/50 last:border-b-0 space-y-3 bg-surface-elevated/30">
                            <div>
                              <label className="text-xs text-text-muted block mb-1">Food Name</label>
                              <input
                                type="text"
                                value={editForm.food_name}
                                onChange={(e) => setEditForm((f) => ({ ...f, food_name: e.target.value }))}
                                className="input text-sm w-full"
                              />
                            </div>
                            <div className="grid grid-cols-2 gap-2">
                              <div>
                                <label className="text-xs text-text-muted block mb-1">Calories</label>
                                <input
                                  type="number"
                                  value={editForm.calories}
                                  onChange={(e) => setEditForm((f) => ({ ...f, calories: Number(e.target.value) || 0 }))}
                                  className="input text-sm w-full"
                                />
                              </div>
                              <div>
                                <label className="text-xs text-text-muted block mb-1">Quantity</label>
                                <input
                                  type="number"
                                  step="0.25"
                                  value={editForm.quantity}
                                  onChange={(e) => setEditForm((f) => ({ ...f, quantity: Number(e.target.value) || 0.25 }))}
                                  className="input text-sm w-full"
                                />
                              </div>
                            </div>
                            <div className="grid grid-cols-3 gap-2">
                              <div>
                                <label className="text-xs text-text-muted block mb-1">Protein (g)</label>
                                <input
                                  type="number"
                                  step="0.1"
                                  value={editForm.protein_g}
                                  onChange={(e) => setEditForm((f) => ({ ...f, protein_g: Number(e.target.value) || 0 }))}
                                  className="input text-sm w-full"
                                />
                              </div>
                              <div>
                                <label className="text-xs text-text-muted block mb-1">Carbs (g)</label>
                                <input
                                  type="number"
                                  step="0.1"
                                  value={editForm.carbs_g}
                                  onChange={(e) => setEditForm((f) => ({ ...f, carbs_g: Number(e.target.value) || 0 }))}
                                  className="input text-sm w-full"
                                />
                              </div>
                              <div>
                                <label className="text-xs text-text-muted block mb-1">Fat (g)</label>
                                <input
                                  type="number"
                                  step="0.1"
                                  value={editForm.fat_g}
                                  onChange={(e) => setEditForm((f) => ({ ...f, fat_g: Number(e.target.value) || 0 }))}
                                  className="input text-sm w-full"
                                />
                              </div>
                            </div>
                            <div className="flex gap-2">
                              <button
                                onClick={() => setEditingEntry(null)}
                                className="btn-secondary flex-1 text-sm py-2"
                              >
                                Cancel
                              </button>
                              <button
                                onClick={() => handleEditSave(entry.id)}
                                disabled={editSaving}
                                className="btn-primary flex-1 text-sm py-2"
                              >
                                {editSaving ? 'Saving...' : 'Save'}
                              </button>
                            </div>
                          </div>
                        ) : (
                          /* Normal display */
                          <div key={entry.id} className="flex items-center justify-between px-4 py-3 border-b border-surface-elevated/50 last:border-b-0">
                            <div className="min-w-0 flex-1">
                              <div className="text-sm font-medium text-text-primary truncate">{entry.food_name}</div>
                              <div className="text-xs text-text-muted">
                                {entry.serving_size} {entry.serving_unit} x {entry.quantity} — P:{Math.round(entry.protein_g)}g C:{Math.round(entry.carbs_g)}g F:{Math.round(entry.fat_g)}g
                              </div>
                              {(entry.fiber_g != null && entry.fiber_g > 0) || (entry.sugar_g != null && entry.sugar_g > 0) ? (
                                <div className="text-xs text-text-muted/70 mt-0.5">
                                  {entry.fiber_g != null && entry.fiber_g > 0 && <span>Fiber: {Math.round(entry.fiber_g * 10) / 10}g</span>}
                                  {entry.fiber_g != null && entry.fiber_g > 0 && entry.sugar_g != null && entry.sugar_g > 0 && <span> &middot; </span>}
                                  {entry.sugar_g != null && entry.sugar_g > 0 && <span>Sugar: {Math.round(entry.sugar_g * 10) / 10}g</span>}
                                </div>
                              ) : null}
                            </div>
                            <div className="flex items-center gap-1 ml-2">
                              <span className="font-mono text-sm text-primary">{entry.calories}</span>
                              <button
                                onClick={() => handleEditStart(entry)}
                                className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-primary transition-colors"
                                title="Edit entry"
                              >
                                <svg width="14" height="14" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
                                  <path d="M11.5 1.5l3 3L5 14H2v-3L11.5 1.5z" />
                                </svg>
                              </button>
                              <button
                                onClick={() => handleDelete(entry.id)}
                                disabled={deletingId === entry.id}
                                className="tap-target w-8 h-8 flex items-center justify-center text-text-muted hover:text-danger transition-colors"
                                title="Delete entry"
                              >
                                <svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
                                  <path d="M4 4l8 8M12 4l-8 8" />
                                </svg>
                              </button>
                            </div>
                          </div>
                        )
                      )
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
