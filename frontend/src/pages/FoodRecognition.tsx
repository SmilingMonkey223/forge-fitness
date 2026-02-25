import { useState, useRef } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '@/services/api'
import type { FoodItem, FoodRecognitionResult } from '@/types'

type RecognitionStatus = 'idle' | 'capturing' | 'uploading' | 'processing' | 'reviewing' | 'logging'

export default function FoodRecognition() {
  const navigate = useNavigate()
  const fileInputRef = useRef<HTMLInputElement>(null)
  const videoRef = useRef<HTMLVideoElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)

  const [status, setStatus] = useState<RecognitionStatus>('idle')
  const [error, setError] = useState<string | null>(null)
  const [capturedImage, setCapturedImage] = useState<Blob | null>(null)
  const [previewUrl, setPreviewUrl] = useState<string | null>(null)
  const [result, setResult] = useState<FoodRecognitionResult | null>(null)
  const [editedItems, setEditedItems] = useState<FoodItem[]>([])
  const [mealType, setMealType] = useState<string>('lunch')
  const [showCamera, setShowCamera] = useState(false)

  const startCamera = async () => {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({
        video: { facingMode: 'environment' }
      })
      if (videoRef.current) {
        videoRef.current.srcObject = stream
        setShowCamera(true)
        setStatus('capturing')
      }
    } catch (err) {
      setError('Unable to access camera. Please allow camera permissions.')
    }
  }

  const stopCamera = () => {
    if (videoRef.current?.srcObject) {
      const tracks = (videoRef.current.srcObject as MediaStream).getTracks()
      tracks.forEach(track => track.stop())
      videoRef.current.srcObject = null
    }
    setShowCamera(false)
  }

  const capturePhoto = () => {
    if (!videoRef.current || !canvasRef.current) return

    const video = videoRef.current
    const canvas = canvasRef.current
    canvas.width = video.videoWidth
    canvas.height = video.videoHeight

    const ctx = canvas.getContext('2d')
    if (!ctx) return

    ctx.drawImage(video, 0, 0)
    canvas.toBlob(blob => {
      if (blob) {
        setCapturedImage(blob)
        setPreviewUrl(URL.createObjectURL(blob))
        stopCamera()
        setStatus('idle')
      }
    }, 'image/jpeg', 0.9)
  }

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0]
    if (file) {
      setCapturedImage(file)
      setPreviewUrl(URL.createObjectURL(file))
    }
  }

  const analyzeFood = async () => {
    if (!capturedImage) return

    setStatus('uploading')
    setError(null)

    try {
      const { task_id } = await api.recognizeFood(capturedImage)
      setStatus('processing')

      // Poll for result
      const pollResult = async (): Promise<FoodRecognitionResult> => {
        const result = await api.getFoodRecognitionResult(task_id)
        if (result.status === 'pending' || result.status === 'processing') {
          await new Promise(resolve => setTimeout(resolve, 500))
          return pollResult()
        }
        return result
      }

      const recognitionResult = await pollResult()
      setResult(recognitionResult)

      if (recognitionResult.status === 'completed' && recognitionResult.items) {
        setEditedItems([...recognitionResult.items])
        setStatus('reviewing')
      } else {
        setError(recognitionResult.error || 'Recognition failed')
        setStatus('idle')
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Recognition failed')
      setStatus('idle')
    }
  }

  const updateItem = (index: number, field: keyof FoodItem, value: string | number) => {
    setEditedItems(items => {
      const updated = [...items]
      updated[index] = { ...updated[index], [field]: value }

      // Recalculate macros if portion changes
      if (field === 'portion_grams' && result?.items?.[index]) {
        const originalItem = result.items[index]
        const ratio = Number(value) / originalItem.portion_grams
        updated[index].calories = Math.round(originalItem.calories * ratio)
        updated[index].protein_g = Math.round(originalItem.protein_g * ratio * 10) / 10
        updated[index].carbs_g = Math.round(originalItem.carbs_g * ratio * 10) / 10
        updated[index].fat_g = Math.round(originalItem.fat_g * ratio * 10) / 10
      }

      return updated
    })
  }

  const removeItem = (index: number) => {
    setEditedItems(items => items.filter((_, i) => i !== index))
  }

  const addItem = () => {
    setEditedItems(items => [...items, {
      name: 'New Item',
      portion_grams: 100,
      calories: 100,
      protein_g: 5,
      carbs_g: 10,
      fat_g: 5
    }])
  }

  const logMeal = async () => {
    if (!result?.task_id || editedItems.length === 0) return

    setStatus('logging')
    try {
      await api.confirmFoodRecognition(result.task_id, editedItems, mealType)
      navigate('/nutrition', { state: { logged: true } })
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to log meal')
      setStatus('reviewing')
    }
  }

  const getConfidenceColor = (confidence?: number) => {
    if (!confidence) return 'bg-gray-200'
    if (confidence >= 0.8) return 'bg-green-500'
    if (confidence >= 0.6) return 'bg-yellow-500'
    return 'bg-red-500'
  }

  const totals = editedItems.reduce((acc, item) => ({
    calories: acc.calories + item.calories,
    protein: acc.protein + item.protein_g,
    carbs: acc.carbs + item.carbs_g,
    fat: acc.fat + item.fat_g
  }), { calories: 0, protein: 0, carbs: 0, fat: 0 })

  return (
    <div className="min-h-screen bg-gray-50 pb-20">
      {/* Header */}
      <div className="bg-white shadow-sm">
        <div className="max-w-lg mx-auto px-4 py-4 flex items-center justify-between">
          <button onClick={() => navigate(-1)} className="text-gray-600">
            ← Back
          </button>
          <h1 className="text-lg font-semibold">AI Food Recognition</h1>
          <div className="w-10" />
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-6">
        {/* Error Display */}
        {error && (
          <div className="mb-4 p-4 bg-red-50 border border-red-200 rounded-lg text-red-700">
            {error}
          </div>
        )}

        {/* Camera/Upload Section */}
        {status === 'idle' && !previewUrl && (
          <div className="space-y-4">
            <div className="bg-white rounded-xl shadow-sm p-6">
              <h2 className="text-lg font-medium mb-4">Take a Photo of Your Meal</h2>
              <p className="text-gray-600 text-sm mb-6">
                Point your camera at your food and we'll identify the items and estimate calories and macros.
              </p>

              <div className="grid grid-cols-2 gap-4">
                <button
                  onClick={startCamera}
                  className="flex flex-col items-center justify-center p-6 border-2 border-dashed border-gray-300 rounded-xl hover:border-blue-500 hover:bg-blue-50 transition-colors"
                >
                  <svg className="w-10 h-10 text-gray-400 mb-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 9a2 2 0 012-2h.93a2 2 0 001.664-.89l.812-1.22A2 2 0 0110.07 4h3.86a2 2 0 011.664.89l.812 1.22A2 2 0 0018.07 7H19a2 2 0 012 2v9a2 2 0 01-2 2H5a2 2 0 01-2-2V9z" />
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M15 13a3 3 0 11-6 0 3 3 0 016 0z" />
                  </svg>
                  <span className="text-sm font-medium text-gray-700">Take Photo</span>
                </button>

                <button
                  onClick={() => fileInputRef.current?.click()}
                  className="flex flex-col items-center justify-center p-6 border-2 border-dashed border-gray-300 rounded-xl hover:border-blue-500 hover:bg-blue-50 transition-colors"
                >
                  <svg className="w-10 h-10 text-gray-400 mb-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z" />
                  </svg>
                  <span className="text-sm font-medium text-gray-700">Choose Photo</span>
                </button>
              </div>

              <input
                ref={fileInputRef}
                type="file"
                accept="image/*"
                onChange={handleFileSelect}
                className="hidden"
              />
            </div>
          </div>
        )}

        {/* Camera View */}
        {showCamera && (
          <div className="relative bg-black rounded-xl overflow-hidden">
            <video
              ref={videoRef}
              autoPlay
              playsInline
              className="w-full"
            />
            <div className="absolute bottom-4 left-0 right-0 flex justify-center gap-4">
              <button
                onClick={stopCamera}
                className="p-3 bg-gray-800/80 text-white rounded-full"
              >
                Cancel
              </button>
              <button
                onClick={capturePhoto}
                className="p-4 bg-white rounded-full"
              >
                <div className="w-12 h-12 rounded-full border-4 border-gray-800" />
              </button>
            </div>
          </div>
        )}

        {/* Preview & Analyze */}
        {previewUrl && status !== 'reviewing' && (
          <div className="space-y-4">
            <div className="relative">
              <img
                src={previewUrl}
                alt="Food preview"
                className="w-full rounded-xl"
              />
              <button
                onClick={() => {
                  setCapturedImage(null)
                  setPreviewUrl(null)
                }}
                className="absolute top-2 right-2 p-2 bg-black/50 text-white rounded-full"
              >
                ✕
              </button>
            </div>

            <button
              onClick={analyzeFood}
              disabled={status === 'uploading' || status === 'processing'}
              className="w-full py-4 bg-blue-600 text-white rounded-xl font-medium disabled:opacity-50"
            >
              {status === 'uploading' && 'Uploading...'}
              {status === 'processing' && 'Analyzing your meal...'}
              {status === 'idle' && 'Analyze Food'}
            </button>
          </div>
        )}

        {/* Review Results */}
        {status === 'reviewing' && result && (
          <div className="space-y-4">
            {/* Preview Image */}
            {previewUrl && (
              <img src={previewUrl} alt="Food" className="w-full rounded-xl" />
            )}

            {/* Confidence Indicator */}
            <div className="flex items-center gap-2 text-sm">
              <span className="text-gray-600">AI Confidence:</span>
              <div className={`w-3 h-3 rounded-full ${getConfidenceColor(result.confidence)}`} />
              <span className="font-medium">
                {result.confidence ? `${Math.round(result.confidence * 100)}%` : 'Unknown'}
              </span>
              <span className="text-gray-400">• Tier {result.tier}</span>
            </div>

            {/* Meal Type Selector */}
            <div className="flex gap-2">
              {['breakfast', 'lunch', 'dinner', 'snack'].map(type => (
                <button
                  key={type}
                  onClick={() => setMealType(type)}
                  className={`px-4 py-2 rounded-full text-sm font-medium capitalize ${
                    mealType === type
                      ? 'bg-blue-600 text-white'
                      : 'bg-gray-100 text-gray-700'
                  }`}
                >
                  {type}
                </button>
              ))}
            </div>

            {/* Editable Food Items */}
            <div className="space-y-3">
              {editedItems.map((item, index) => (
                <div key={index} className="bg-white rounded-xl shadow-sm p-4">
                  <div className="flex justify-between items-start mb-3">
                    <input
                      type="text"
                      value={item.name}
                      onChange={e => updateItem(index, 'name', e.target.value)}
                      className="text-lg font-medium bg-transparent border-b border-transparent hover:border-gray-300 focus:border-blue-500 focus:outline-none"
                    />
                    <button
                      onClick={() => removeItem(index)}
                      className="text-red-500 hover:text-red-700"
                    >
                      Remove
                    </button>
                  </div>

                  <div className="grid grid-cols-2 gap-3 text-sm">
                    <div>
                      <label className="text-gray-500">Portion (g)</label>
                      <input
                        type="number"
                        value={item.portion_grams}
                        onChange={e => updateItem(index, 'portion_grams', Number(e.target.value))}
                        className="w-full mt-1 px-3 py-2 border rounded-lg"
                      />
                    </div>
                    <div>
                      <label className="text-gray-500">Calories</label>
                      <input
                        type="number"
                        value={item.calories}
                        onChange={e => updateItem(index, 'calories', Number(e.target.value))}
                        className="w-full mt-1 px-3 py-2 border rounded-lg"
                      />
                    </div>
                    <div>
                      <label className="text-gray-500">Protein (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={item.protein_g}
                        onChange={e => updateItem(index, 'protein_g', Number(e.target.value))}
                        className="w-full mt-1 px-3 py-2 border rounded-lg"
                      />
                    </div>
                    <div>
                      <label className="text-gray-500">Carbs (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={item.carbs_g}
                        onChange={e => updateItem(index, 'carbs_g', Number(e.target.value))}
                        className="w-full mt-1 px-3 py-2 border rounded-lg"
                      />
                    </div>
                    <div>
                      <label className="text-gray-500">Fat (g)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={item.fat_g}
                        onChange={e => updateItem(index, 'fat_g', Number(e.target.value))}
                        className="w-full mt-1 px-3 py-2 border rounded-lg"
                      />
                    </div>
                  </div>
                </div>
              ))}

              <button
                onClick={addItem}
                className="w-full py-3 border-2 border-dashed border-gray-300 rounded-xl text-gray-600 hover:border-blue-500 hover:text-blue-600"
              >
                + Add Item
              </button>
            </div>

            {/* Totals */}
            <div className="bg-blue-50 rounded-xl p-4">
              <h3 className="font-medium text-blue-900 mb-2">Meal Totals</h3>
              <div className="grid grid-cols-4 gap-2 text-center">
                <div>
                  <div className="text-2xl font-bold text-blue-900">{Math.round(totals.calories)}</div>
                  <div className="text-xs text-blue-700">kcal</div>
                </div>
                <div>
                  <div className="text-2xl font-bold text-blue-900">{Math.round(totals.protein)}</div>
                  <div className="text-xs text-blue-700">protein</div>
                </div>
                <div>
                  <div className="text-2xl font-bold text-blue-900">{Math.round(totals.carbs)}</div>
                  <div className="text-xs text-blue-700">carbs</div>
                </div>
                <div>
                  <div className="text-2xl font-bold text-blue-900">{Math.round(totals.fat)}</div>
                  <div className="text-xs text-blue-700">fat</div>
                </div>
              </div>
            </div>

            {/* Log Button */}
            <button
              onClick={logMeal}
              disabled={status === 'logging' || editedItems.length === 0}
              className="w-full py-4 bg-green-600 text-white rounded-xl font-medium disabled:opacity-50"
            >
              {status === 'logging' ? 'Logging...' : 'Log Meal'}
            </button>
          </div>
        )}
      </div>

      {/* Hidden canvas for photo capture */}
      <canvas ref={canvasRef} className="hidden" />
    </div>
  )
}
