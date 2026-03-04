import { useState, useEffect, useRef, useCallback } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type { RecentFood, USDAFood, CustomFood } from '@/types'

type Tab = 'recent' | 'search' | 'custom' | 'scan'

interface FoodSearchProps {
  open: boolean
  mealType: string
  onClose: () => void
  onLogged: () => void
}

interface PortionEditorProps {
  food: { name: string; brand?: string; serving_size: number; serving_unit: string; calories: number; protein_g: number; carbs_g: number; fat_g: number }
  mealType: string
  onLog: () => void
  onCancel: () => void
}

function PortionEditor({ food, mealType, onLog, onCancel }: PortionEditorProps) {
  const [quantity, setQuantity] = useState(1)
  const [logging, setLogging] = useState(false)
  const [logError, setLogError] = useState<string | null>(null)

  const cals = Math.round(food.calories * quantity)
  const protein = Math.round(food.protein_g * quantity * 10) / 10
  const carbs = Math.round(food.carbs_g * quantity * 10) / 10
  const fat = Math.round(food.fat_g * quantity * 10) / 10

  const handleLog = async () => {
    setLogging(true)
    setLogError(null)
    try {
      await api.logNutrition({
        meal_type: mealType,
        food_name: food.name,
        serving_size: food.serving_size,
        serving_unit: food.serving_unit,
        quantity,
        calories: cals,
        protein_g: protein,
        carbs_g: carbs,
        fat_g: fat,
      })
      onLog()
    } catch {
      setLogError('Failed to log food. Please try again.')
    } finally {
      setLogging(false)
    }
  }

  return (
    <div className="p-4 space-y-4">
      {logError && (
        <div className="bg-danger/10 text-danger border border-danger/30 rounded-card px-4 py-3 flex items-center justify-between">
          <span className="text-sm">{logError}</span>
          <button onClick={() => setLogError(null)} className="ml-2 text-danger hover:text-danger/70">
            <svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
              <path d="M4 4l8 8M12 4l-8 8" />
            </svg>
          </button>
        </div>
      )}
      <div>
        <h3 className="font-bold text-text-primary">{food.name}</h3>
        {food.brand && <div className="text-sm text-text-muted">{food.brand}</div>}
      </div>

      <div>
        <label className="label">Serving: {food.serving_size} {food.serving_unit}</label>
        <div className="flex items-center gap-3">
          <button
            onClick={() => setQuantity(Math.max(0.25, quantity - 0.25))}
            className="tap-target w-10 h-10 flex items-center justify-center rounded-full bg-surface-elevated text-text-secondary text-lg"
          >-</button>
          <input
            type="number"
            value={quantity}
            onChange={(e) => setQuantity(Math.max(0.25, Number(e.target.value) || 0.25))}
            step="0.25"
            className="w-20 text-center text-xl font-mono font-bold bg-transparent text-text-primary border-b-2 border-text-muted focus:border-primary
                       [appearance:textfield] [&::-webkit-outer-spin-button]:appearance-none [&::-webkit-inner-spin-button]:appearance-none"
          />
          <button
            onClick={() => setQuantity(quantity + 0.25)}
            className="tap-target w-10 h-10 flex items-center justify-center rounded-full bg-surface-elevated text-text-secondary text-lg"
          >+</button>
          <span className="text-text-muted text-sm">servings</span>
        </div>
      </div>

      <div className="grid grid-cols-4 gap-3 bg-surface-elevated rounded-lg p-3">
        <div className="text-center">
          <div className="text-xs text-text-muted">Calories</div>
          <div className="font-bold font-mono text-primary">{cals}</div>
        </div>
        <div className="text-center">
          <div className="text-xs text-text-muted">Protein</div>
          <div className="font-bold font-mono text-success">{protein}g</div>
        </div>
        <div className="text-center">
          <div className="text-xs text-text-muted">Carbs</div>
          <div className="font-bold font-mono text-warning">{carbs}g</div>
        </div>
        <div className="text-center">
          <div className="text-xs text-text-muted">Fat</div>
          <div className="font-bold font-mono text-danger">{fat}g</div>
        </div>
      </div>

      <div className="flex gap-3">
        <button onClick={onCancel} className="btn-secondary flex-1">Cancel</button>
        <button onClick={handleLog} disabled={logging} className="btn-primary flex-1">
          {logging ? 'Logging...' : 'Log Food'}
        </button>
      </div>
    </div>
  )
}

export default function FoodSearch({ open, mealType, onClose, onLogged }: FoodSearchProps) {
  const navigate = useNavigate()
  const [tab, setTab] = useState<Tab>('recent')
  const [search, setSearch] = useState('')
  const [recentFoods, setRecentFoods] = useState<RecentFood[]>([])
  const [searchResults, setSearchResults] = useState<USDAFood[]>([])
  const [customFoods, setCustomFoods] = useState<CustomFood[]>([])
  const [loading, setLoading] = useState(false)
  const [selectedFood, setSelectedFood] = useState<PortionEditorProps['food'] | null>(null)
  const debounceRef = useRef<ReturnType<typeof setTimeout> | null>(null)
  const [showCreateCustom, setShowCreateCustom] = useState(false)
  const [customFormSaving, setCustomFormSaving] = useState(false)
  const [customFormError, setCustomFormError] = useState<string | null>(null)
  const [customForm, setCustomForm] = useState({
    name: '',
    brand: '',
    serving_size: 100,
    serving_unit: 'g',
    calories: 0,
    protein_g: 0,
    carbs_g: 0,
    fat_g: 0,
  })

  const fetchRecent = useCallback(async () => {
    try {
      const foods = await api.getRecentFoods()
      setRecentFoods(foods)
    } catch { /* ignore */ }
  }, [])

  const fetchCustom = useCallback(async () => {
    try {
      const foods = await api.getCustomFoods()
      setCustomFoods(foods)
    } catch { /* ignore */ }
  }, [])

  useEffect(() => {
    if (open) {
      fetchRecent()
      fetchCustom()
      setSearch('')
      setSearchResults([])
      setSelectedFood(null)
      setShowCreateCustom(false)
      setCustomForm({ name: '', brand: '', serving_size: 100, serving_unit: 'g', calories: 0, protein_g: 0, carbs_g: 0, fat_g: 0 })
      setCustomFormError(null)
    }
  }, [open, fetchRecent, fetchCustom])

  useEffect(() => {
    if (tab !== 'search' || !search.trim()) return
    if (debounceRef.current) clearTimeout(debounceRef.current)
    debounceRef.current = setTimeout(async () => {
      setLoading(true)
      try {
        const results = await api.searchFoods(search)
        setSearchResults(results)
      } catch { setSearchResults([]) }
      finally { setLoading(false) }
    }, 300)
    return () => { if (debounceRef.current) clearTimeout(debounceRef.current) }
  }, [search, tab])

  if (!open) return null

  const handleSelectRecent = (food: RecentFood) => {
    setSelectedFood({
      name: food.food_name,
      brand: food.brand,
      serving_size: food.serving_size,
      serving_unit: food.serving_unit,
      calories: food.calories,
      protein_g: food.protein_g,
      carbs_g: food.carbs_g,
      fat_g: food.fat_g,
    })
  }

  const handleSelectUSDA = (food: USDAFood) => {
    setSelectedFood({
      name: food.description,
      brand: food.brand_owner,
      serving_size: food.serving_size || 100,
      serving_unit: food.serving_unit || 'g',
      calories: food.calories,
      protein_g: food.protein_g,
      carbs_g: food.carbs_g,
      fat_g: food.fat_g,
    })
  }

  const handleSelectCustom = (food: CustomFood) => {
    setSelectedFood({
      name: food.name,
      brand: food.brand,
      serving_size: food.serving_size,
      serving_unit: food.serving_unit,
      calories: food.calories,
      protein_g: food.protein_g,
      carbs_g: food.carbs_g,
      fat_g: food.fat_g,
    })
  }

  const handleLogged = () => {
    setSelectedFood(null)
    onLogged()
    onClose()
  }

  const resetCustomForm = () => {
    setCustomForm({ name: '', brand: '', serving_size: 100, serving_unit: 'g', calories: 0, protein_g: 0, carbs_g: 0, fat_g: 0 })
    setCustomFormError(null)
  }

  const handleCreateCustomFood = async () => {
    if (!customForm.name.trim()) {
      setCustomFormError('Food name is required.')
      return
    }
    setCustomFormSaving(true)
    setCustomFormError(null)
    try {
      await api.createCustomFood({
        name: customForm.name.trim(),
        brand: customForm.brand.trim() || undefined,
        serving_size: customForm.serving_size,
        serving_unit: customForm.serving_unit,
        calories: customForm.calories,
        protein_g: customForm.protein_g,
        carbs_g: customForm.carbs_g,
        fat_g: customForm.fat_g,
      })
      resetCustomForm()
      setShowCreateCustom(false)
      fetchCustom()
    } catch {
      setCustomFormError('Failed to create custom food. Please try again.')
    } finally {
      setCustomFormSaving(false)
    }
  }

  const tabs: { key: Tab; label: string }[] = [
    { key: 'recent', label: 'Recent' },
    { key: 'search', label: 'Search' },
    { key: 'custom', label: 'Custom' },
    { key: 'scan', label: 'Scan' },
  ]

  return (
    <div className="fixed inset-0 z-40 flex flex-col bg-background">
      <div className="flex items-center gap-3 px-4 py-3 bg-surface border-b border-text-muted">
        <button onClick={onClose} className="tap-target p-2 text-text-secondary hover:text-text-primary">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
        </button>
        <h2 className="text-lg font-bold flex-1">Add Food</h2>
      </div>

      <div className="flex bg-surface border-b border-text-muted/30">
        {tabs.map((t) => (
          <button
            key={t.key}
            onClick={() => {
              if (t.key === 'scan') {
                onClose()
                navigate('/food-recognition')
                return
              }
              setTab(t.key)
              setSelectedFood(null)
            }}
            className={`flex-1 py-3 text-sm font-medium transition-colors ${
              tab === t.key
                ? 'text-primary border-b-2 border-primary'
                : 'text-text-muted hover:text-text-secondary'
            }`}
          >
            {t.label}
          </button>
        ))}
      </div>

      {selectedFood ? (
        <PortionEditor
          food={selectedFood}
          mealType={mealType}
          onLog={handleLogged}
          onCancel={() => setSelectedFood(null)}
        />
      ) : (
        <div className="flex-1 overflow-y-auto">
          {tab === 'search' && (
            <div className="p-4">
              <input
                type="text"
                placeholder="Search USDA food database..."
                value={search}
                onChange={(e) => setSearch(e.target.value)}
                className="input"
                autoFocus
              />
            </div>
          )}

          {tab === 'recent' && (
            <div className="p-4 space-y-1">
              {recentFoods.length === 0 ? (
                <div className="text-center py-12 text-text-muted">
                  No recent foods yet. Start logging to see them here.
                </div>
              ) : (
                recentFoods.map((food, i) => (
                  <button
                    key={i}
                    onClick={() => handleSelectRecent(food)}
                    className="tap-target w-full flex items-center justify-between p-3 rounded-lg hover:bg-surface-elevated text-left"
                  >
                    <div className="min-w-0 flex-1">
                      <div className="font-medium text-text-primary truncate">{food.food_name}</div>
                      <div className="text-xs text-text-muted">{food.serving_size} {food.serving_unit}</div>
                    </div>
                    <div className="text-sm font-mono text-primary ml-3">{food.calories} cal</div>
                  </button>
                ))
              )}
            </div>
          )}

          {tab === 'search' && (
            <div className="px-4 space-y-1">
              {loading ? (
                <div className="text-center py-12 text-text-muted">Searching...</div>
              ) : search.trim() && searchResults.length === 0 ? (
                <div className="text-center py-12 text-text-muted">No results found</div>
              ) : (
                searchResults.map((food) => (
                  <button
                    key={food.fdc_id}
                    onClick={() => handleSelectUSDA(food)}
                    className="tap-target w-full flex items-center justify-between p-3 rounded-lg hover:bg-surface-elevated text-left"
                  >
                    <div className="min-w-0 flex-1">
                      <div className="font-medium text-text-primary truncate">{food.description}</div>
                      {food.brand_owner && (
                        <div className="text-xs text-text-muted truncate">{food.brand_owner}</div>
                      )}
                    </div>
                    <div className="text-sm font-mono text-primary ml-3">{food.calories} cal</div>
                  </button>
                ))
              )}
            </div>
          )}

          {tab === 'custom' && (
            <div className="p-4 space-y-2">
              {/* Create custom food toggle button */}
              {!showCreateCustom && (
                <button
                  onClick={() => { resetCustomForm(); setShowCreateCustom(true) }}
                  className="tap-target w-full flex items-center justify-center gap-2 p-3 rounded-lg border border-dashed border-primary/40 text-primary hover:bg-primary/5 transition-colors"
                >
                  <svg width="18" height="18" viewBox="0 0 18 18" fill="none" stroke="currentColor" strokeWidth="2">
                    <path d="M9 3v12M3 9h12" />
                  </svg>
                  <span className="text-sm font-medium">Create Custom Food</span>
                </button>
              )}

              {/* Inline create custom food form */}
              {showCreateCustom && (
                <div className="rounded-lg border border-border bg-surface p-4 space-y-3">
                  <div className="flex items-center justify-between mb-1">
                    <h3 className="font-bold text-text-primary text-sm">New Custom Food</h3>
                    <button
                      onClick={() => { setShowCreateCustom(false); resetCustomForm() }}
                      className="text-text-muted hover:text-text-primary"
                    >
                      <svg width="16" height="16" viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="1.5">
                        <path d="M4 4l8 8M12 4l-8 8" />
                      </svg>
                    </button>
                  </div>

                  {customFormError && (
                    <div className="bg-danger/10 text-danger border border-danger/30 rounded-card px-3 py-2 text-sm">
                      {customFormError}
                    </div>
                  )}

                  <div>
                    <label className="text-xs text-text-muted block mb-1">Name *</label>
                    <input
                      type="text"
                      value={customForm.name}
                      onChange={(e) => setCustomForm((f) => ({ ...f, name: e.target.value }))}
                      placeholder="e.g. Homemade Granola"
                      className="input text-sm w-full"
                    />
                  </div>
                  <div>
                    <label className="text-xs text-text-muted block mb-1">Brand (optional)</label>
                    <input
                      type="text"
                      value={customForm.brand}
                      onChange={(e) => setCustomForm((f) => ({ ...f, brand: e.target.value }))}
                      placeholder="e.g. My Kitchen"
                      className="input text-sm w-full"
                    />
                  </div>
                  <div className="grid grid-cols-2 gap-2">
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Serving Size</label>
                      <input
                        type="number"
                        value={customForm.serving_size}
                        onChange={(e) => setCustomForm((f) => ({ ...f, serving_size: Number(e.target.value) || 0 }))}
                        className="input text-sm w-full"
                      />
                    </div>
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Serving Unit</label>
                      <input
                        type="text"
                        value={customForm.serving_unit}
                        onChange={(e) => setCustomForm((f) => ({ ...f, serving_unit: e.target.value }))}
                        placeholder="g, ml, oz..."
                        className="input text-sm w-full"
                      />
                    </div>
                  </div>
                  <div className="grid grid-cols-2 gap-2">
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Calories</label>
                      <input
                        type="number"
                        value={customForm.calories}
                        onChange={(e) => setCustomForm((f) => ({ ...f, calories: Number(e.target.value) || 0 }))}
                        className="input text-sm w-full"
                      />
                    </div>
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Protein (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={customForm.protein_g}
                        onChange={(e) => setCustomForm((f) => ({ ...f, protein_g: Number(e.target.value) || 0 }))}
                        className="input text-sm w-full"
                      />
                    </div>
                  </div>
                  <div className="grid grid-cols-2 gap-2">
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Carbs (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={customForm.carbs_g}
                        onChange={(e) => setCustomForm((f) => ({ ...f, carbs_g: Number(e.target.value) || 0 }))}
                        className="input text-sm w-full"
                      />
                    </div>
                    <div>
                      <label className="text-xs text-text-muted block mb-1">Fat (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={customForm.fat_g}
                        onChange={(e) => setCustomForm((f) => ({ ...f, fat_g: Number(e.target.value) || 0 }))}
                        className="input text-sm w-full"
                      />
                    </div>
                  </div>
                  <div className="flex gap-2 pt-1">
                    <button
                      onClick={() => { setShowCreateCustom(false); resetCustomForm() }}
                      className="btn-secondary flex-1 text-sm py-2"
                    >
                      Cancel
                    </button>
                    <button
                      onClick={handleCreateCustomFood}
                      disabled={customFormSaving}
                      className="btn-primary flex-1 text-sm py-2"
                    >
                      {customFormSaving ? 'Creating...' : 'Create Food'}
                    </button>
                  </div>
                </div>
              )}

              {/* Existing custom foods list */}
              {customFoods.length === 0 && !showCreateCustom ? (
                <div className="text-center py-12 text-text-muted">No custom foods yet.</div>
              ) : (
                customFoods.map((food) => (
                  <button
                    key={food.id}
                    onClick={() => handleSelectCustom(food)}
                    className="tap-target w-full flex items-center justify-between p-3 rounded-lg hover:bg-surface-elevated text-left"
                  >
                    <div className="min-w-0 flex-1">
                      <div className="font-medium text-text-primary truncate">{food.name}</div>
                      {food.brand && <div className="text-xs text-text-muted">{food.brand}</div>}
                    </div>
                    <div className="text-sm font-mono text-primary ml-3">{food.calories} cal</div>
                  </button>
                ))
              )}
            </div>
          )}
        </div>
      )}
    </div>
  )
}
