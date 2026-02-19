/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        background: '#0A0A0F',
        surface: '#14141F',
        'surface-elevated': '#1E1E2E',
        primary: '#6C5CE7',
        success: '#00D68F',
        warning: '#FFB800',
        danger: '#FF5252',
        celebration: '#FFD700',
        'text-primary': '#F0F0F0',
        'text-secondary': '#8888A0',
        'text-muted': '#555570',
      },
      fontFamily: {
        sans: ['Inter', 'system-ui', 'sans-serif'],
        mono: ['JetBrains Mono', 'monospace'],
      },
      borderRadius: {
        'card': '8px',
        'modal': '12px',
        'button': '24px',
      },
      boxShadow: {
        'card': '0 2px 8px rgba(0,0,0,0.3)',
      },
    },
  },
  plugins: [],
}
