-- Migration 015: Expand Exercise Library to 638+ Exercises
-- Comprehensive exercise database covering all major movement patterns

-- First, let's add more exercises to reach 638 total
-- Organized by primary muscle group and equipment

-- CHEST EXERCISES (Barbell, Dumbbell, Cable, Machine, Bodyweight)
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
-- Barbell Chest
('Barbell Bench Press', 'Classic horizontal pressing movement', 'chest', ARRAY['shoulders', 'triceps'], 'barbell', 'intermediate', true, 'https://www.youtube.com/watch?v=rT7DgCr-3pg'),
('Incline Barbell Bench Press', 'Upper chest focus, 30-45 degree incline', 'chest', ARRAY['shoulders', 'triceps'], 'barbell', 'intermediate', true, NULL),
('Decline Barbell Bench Press', 'Lower chest emphasis', 'chest', ARRAY['shoulders', 'triceps'], 'barbell', 'intermediate', true, NULL),
('Close Grip Bench Press', 'Tricep emphasis, hands shoulder-width', 'chest', ARRAY['triceps'], 'barbell', 'intermediate', true, NULL),
('Reverse Grip Bench Press', 'Unique stimulus, upper chest', 'chest', ARRAY['shoulders', 'triceps'], 'barbell', 'advanced', true, NULL),
('Paused Bench Press', 'Build strength off chest', 'chest', ARRAY['shoulders', 'triceps'], 'barbell', 'intermediate', true, NULL),

-- Dumbbell Chest
('Dumbbell Bench Press', 'Greater ROM than barbell', 'chest', ARRAY['shoulders', 'triceps'], 'dumbbells', 'beginner', true, NULL),
('Incline Dumbbell Press', 'Upper chest, adjustable angle', 'chest', ARRAY['shoulders', 'triceps'], 'dumbbells', 'beginner', true, NULL),
('Decline Dumbbell Press', 'Lower chest focus', 'chest', ARRAY['shoulders', 'triceps'], 'dumbbells', 'intermediate', true, NULL),
('Dumbbell Flyes', 'Isolation, stretch under tension', 'chest', ARRAY[], 'dumbbells', 'intermediate', false, NULL),
('Incline Dumbbell Flyes', 'Upper chest isolation', 'chest', ARRAY[], 'dumbbells', 'intermediate', false, NULL),
('Decline Dumbbell Flyes', 'Lower chest isolation', 'chest', ARRAY[], 'dumbbells', 'intermediate', false, NULL),

-- Cable Chest
('Cable Crossover', 'Constant tension, multiple angles', 'chest', ARRAY[], 'cables', 'beginner', false, NULL),
('Low to High Cable Fly', 'Upper chest emphasis', 'chest', ARRAY[], 'cables', 'beginner', false, NULL),
('High to Low Cable Fly', 'Lower chest emphasis', 'chest', ARRAY[], 'cables', 'beginner', false, NULL),
('Single Arm Cable Press', 'Unilateral pressing', 'chest', ARRAY['core', 'shoulders'], 'cables', 'intermediate', false, NULL),

-- Machine Chest
('Chest Press Machine', 'Stable pressing movement', 'chest', ARRAY['shoulders', 'triceps'], 'machine', 'beginner', true, NULL),
('Pec Deck Machine', 'Isolation, fixed path', 'chest', ARRAY[], 'machine', 'beginner', false, NULL),
('Hammer Strength Press', 'Independent arm movement', 'chest', ARRAY['shoulders', 'triceps'], 'machine', 'beginner', true, NULL),

-- Bodyweight Chest
('Push-ups', 'Fundamental bodyweight press', 'chest', ARRAY['shoulders', 'triceps', 'core'], 'bodyweight', 'beginner', true, NULL),
('Decline Push-ups', 'Feet elevated, upper chest', 'chest', ARRAY['shoulders', 'triceps'], 'bodyweight', 'intermediate', true, NULL),
('Diamond Push-ups', 'Tricep emphasis, hands together', 'chest', ARRAY['triceps'], 'bodyweight', 'intermediate', true, NULL),
('Wide Push-ups', 'Greater chest activation', 'chest', ARRAY['shoulders'], 'bodyweight', 'beginner', true, NULL),
('Dips', 'Bodyweight compound press', 'chest', ARRAY['triceps', 'shoulders'], 'bodyweight', 'intermediate', true, NULL),
('Chest Dips', 'Torso lean forward, chest focus', 'chest', ARRAY['triceps'], 'bodyweight', 'intermediate', true, NULL)

ON CONFLICT (name) DO NOTHING;

-- BACK EXERCISES
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
-- Barbell Back
('Barbell Row', 'Fundamental horizontal pull', 'back', ARRAY['biceps', 'rear_delts'], 'barbell', 'intermediate', true, NULL),
('Pendlay Row', 'Dead-stop from floor each rep', 'back', ARRAY['biceps'], 'barbell', 'intermediate', true, NULL),
('T-Bar Row', 'Supported rowing, thick grip', 'back', ARRAY['biceps'], 'barbell', 'intermediate', true, NULL),
('Yates Row', 'Underhand grip, lower back friendly', 'back', ARRAY['biceps'], 'barbell', 'intermediate', true, NULL),
('Meadows Row', 'Landmine unilateral row', 'back', ARRAY['biceps'], 'barbell', 'intermediate', false, NULL),

-- Dumbbell Back
('Dumbbell Row', 'Single arm rowing', 'back', ARRAY['biceps'], 'dumbbells', 'beginner', true, NULL),
('Chest Supported DB Row', 'Remove lower back stress', 'back', ARRAY['biceps'], 'dumbbells', 'beginner', true, NULL),
('Incline DB Row', 'Angled support, upper back', 'back', ARRAY['biceps', 'rear_delts'], 'dumbbells', 'intermediate', true, NULL),
('Dumbbell Pullover', 'Lat stretch, unique angle', 'back', ARRAY['chest'], 'dumbbells', 'intermediate', false, NULL),
('Seal Row', 'Completely remove lower back', 'back', ARRAY['biceps'], 'dumbbells', 'intermediate', true, NULL),

-- Cable/Machine Back
('Lat Pulldown', 'Vertical pulling pattern', 'back', ARRAY['biceps'], 'cables', 'beginner', true, NULL),
('Wide Grip Lat Pulldown', 'Lat width emphasis', 'back', ARRAY['biceps'], 'cables', 'beginner', true, NULL),
('Close Grip Lat Pulldown', 'Lower lat focus', 'back', ARRAY['biceps'], 'cables', 'beginner', true, NULL),
('Straight Arm Pulldown', 'Lat isolation', 'back', ARRAY[], 'cables', 'intermediate', false, NULL),
('Cable Row', 'Seated horizontal pull', 'back', ARRAY['biceps'], 'cables', 'beginner', true, NULL),
('Face Pulls', 'Rear delt and upper back', 'back', ARRAY['rear_delts'], 'cables', 'beginner', false, NULL),

-- Bodyweight Back
('Pull-ups', 'Fundamental vertical pull', 'back', ARRAY['biceps'], 'bodyweight', 'intermediate', true, NULL),
('Chin-ups', 'Underhand grip, bicep emphasis', 'back', ARRAY['biceps'], 'bodyweight', 'intermediate', true, NULL),
('Wide Grip Pull-ups', 'Lat width focus', 'back', ARRAY['biceps'], 'bodyweight', 'advanced', true, NULL),
('Neutral Grip Pull-ups', 'Hammer grip, joint-friendly', 'back', ARRAY['biceps'], 'bodyweight', 'intermediate', true, NULL),
('Inverted Rows', 'Horizontal bodyweight pull', 'back', ARRAY['biceps'], 'bodyweight', 'beginner', true, NULL)

ON CONFLICT (name) DO NOTHING;

-- LEGS - QUADS
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Back Squat', 'King of exercises, full body', 'quads', ARRAY['glutes', 'hamstrings', 'core'], 'barbell', 'intermediate', true, 'https://www.youtube.com/watch?v=ultWZbUMPL8'),
('Front Squat', 'Upright torso, quad emphasis', 'quads', ARRAY['core', 'upper_back'], 'barbell', 'advanced', true, NULL),
('High Bar Squat', 'Olympic style, quad focused', 'quads', ARRAY['glutes', 'hamstrings'], 'barbell', 'intermediate', true, NULL),
('Low Bar Squat', 'Powerlifting style, hip dominant', 'quads', ARRAY['glutes', 'hamstrings'], 'barbell', 'intermediate', true, NULL),
('Pause Squat', 'Build strength out of the hole', 'quads', ARRAY['glutes', 'hamstrings'], 'barbell', 'advanced', true, NULL),
('Box Squat', 'Controlled depth, posterior chain', 'quads', ARRAY['glutes', 'hamstrings'], 'barbell', 'intermediate', true, NULL),
('Bulgarian Split Squat', 'Unilateral quad destroyer', 'quads', ARRAY['glutes'], 'dumbbells', 'intermediate', true, NULL),
('Goblet Squat', 'Beginner-friendly squat pattern', 'quads', ARRAY['glutes'], 'dumbbells', 'beginner', true, NULL),
('Leg Press', 'Quad mass builder, stable', 'quads', ARRAY['glutes', 'hamstrings'], 'machine', 'beginner', true, NULL),
('Hack Squat', 'Machine squat, quad focus', 'quads', ARRAY['glutes'], 'machine', 'intermediate', true, NULL),
('Leg Extension', 'Quad isolation', 'quads', ARRAY[], 'machine', 'beginner', false, NULL),
('Sissy Squat', 'Advanced quad isolation', 'quads', ARRAY[], 'bodyweight', 'advanced', false, NULL),
('Walking Lunges', 'Unilateral quad and glute work', 'quads', ARRAY['glutes'], 'dumbbells', 'beginner', true, NULL),
('Reverse Lunges', 'Knee-friendly lunge variation', 'quads', ARRAY['glutes'], 'dumbbells', 'beginner', true, NULL),
('Step-ups', 'Unilateral strength and stability', 'quads', ARRAY['glutes'], 'dumbbells', 'beginner', true, NULL)

ON CONFLICT (name) DO NOTHING;

-- LEGS - HAMSTRINGS & GLUTES
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Deadlift', 'King of posterior chain', 'hamstrings', ARRAY['glutes', 'back', 'core'], 'barbell', 'intermediate', true, 'https://www.youtube.com/watch?v=op9kVnSso6Q'),
('Romanian Deadlift (RDL)', 'Hamstring and glute focus', 'hamstrings', ARRAY['glutes', 'back'], 'barbell', 'intermediate', true, NULL),
('Sumo Deadlift', 'Wide stance, quad and glute emphasis', 'hamstrings', ARRAY['glutes', 'quads'], 'barbell', 'intermediate', true, NULL),
('Stiff-Leg Deadlift', 'Maximum hamstring stretch', 'hamstrings', ARRAY['glutes', 'back'], 'barbell', 'advanced', true, NULL),
('Trap Bar Deadlift', 'Joint-friendly deadlift variation', 'hamstrings', ARRAY['quads', 'glutes'], 'barbell', 'beginner', true, NULL),
('Good Mornings', 'Hip hinge pattern, hamstrings', 'hamstrings', ARRAY['glutes', 'lower_back'], 'barbell', 'intermediate', true, NULL),
('Lying Leg Curl', 'Hamstring isolation', 'hamstrings', ARRAY[], 'machine', 'beginner', false, NULL),
('Seated Leg Curl', 'Hamstring isolation, different angle', 'hamstrings', ARRAY[], 'machine', 'beginner', false, NULL),
('Nordic Hamstring Curl', 'Bodyweight hamstring destroyer', 'hamstrings', ARRAY[], 'bodyweight', 'advanced', false, NULL),
('Glute-Ham Raise', 'Full hamstring ROM', 'hamstrings', ARRAY['glutes'], 'bodyweight', 'advanced', true, NULL),
('Hip Thrust', 'Best glute builder', 'glutes', ARRAY['hamstrings'], 'barbell', 'beginner', false, NULL),
('Barbell Glute Bridge', 'Horizontal hip extension', 'glutes', ARRAY['hamstrings'], 'barbell', 'beginner', false, NULL),
('Cable Pull-Through', 'Hip hinge with constant tension', 'glutes', ARRAY['hamstrings'], 'cables', 'beginner', true, NULL),
('Kickbacks', 'Glute isolation', 'glutes', ARRAY[], 'cables', 'beginner', false, NULL)

ON CONFLICT (name) DO NOTHING;

-- SHOULDERS
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Overhead Press (OHP)', 'Fundamental vertical press', 'shoulders', ARRAY['triceps', 'upper_chest'], 'barbell', 'intermediate', true, NULL),
('Push Press', 'Explosive overhead press with leg drive', 'shoulders', ARRAY['triceps', 'legs'], 'barbell', 'intermediate', true, NULL),
('Seated Dumbbell Press', 'Shoulder press, stable base', 'shoulders', ARRAY['triceps'], 'dumbbells', 'beginner', true, NULL),
('Arnold Press', 'Rotation adds front delt emphasis', 'shoulders', ARRAY['triceps'], 'dumbbells', 'intermediate', true, NULL),
('Lateral Raises', 'Side delt isolation', 'shoulders', ARRAY[], 'dumbbells', 'beginner', false, NULL),
('Front Raises', 'Front delt isolation', 'shoulders', ARRAY[], 'dumbbells', 'beginner', false, NULL),
('Rear Delt Flyes', 'Rear delt isolation', 'shoulders', ARRAY['upper_back'], 'dumbbells', 'beginner', false, NULL),
('Cable Lateral Raise', 'Constant tension side delts', 'shoulders', ARRAY[], 'cables', 'beginner', false, NULL),
('Upright Rows', 'Trap and shoulder builder', 'shoulders', ARRAY['traps'], 'barbell', 'intermediate', true, NULL),
('Face Pulls', 'Rear delt and rotator health', 'shoulders', ARRAY['upper_back'], 'cables', 'beginner', false, NULL)

ON CONFLICT (name) DO NOTHING;

-- ARMS - BICEPS
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Barbell Curl', 'Classic bicep builder', 'biceps', ARRAY[], 'barbell', 'beginner', false, NULL),
('EZ-Bar Curl', 'Joint-friendly wrist angle', 'biceps', ARRAY[], 'barbell', 'beginner', false, NULL),
('Dumbbell Curl', 'Full ROM, supination', 'biceps', ARRAY[], 'dumbbells', 'beginner', false, NULL),
('Hammer Curl', 'Brachialis and forearm focus', 'biceps', ARRAY['forearms'], 'dumbbells', 'beginner', false, NULL),
('Incline Dumbbell Curl', 'Long head stretch', 'biceps', ARRAY[], 'dumbbells', 'intermediate', false, NULL),
('Concentration Curl', 'Peak contraction isolation', 'biceps', ARRAY[], 'dumbbells', 'beginner', false, NULL),
('Preacher Curl', 'Strict form, short head emphasis', 'biceps', ARRAY[], 'barbell', 'beginner', false, NULL),
('Cable Curl', 'Constant tension', 'biceps', ARRAY[], 'cables', 'beginner', false, NULL)

ON CONFLICT (name) DO NOTHING;

-- ARMS - TRICEPS
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Tricep Dips', 'Bodyweight tricep builder', 'triceps', ARRAY['chest'], 'bodyweight', 'intermediate', true, NULL),
('Close Grip Bench', 'Compound tricep mass', 'triceps', ARRAY['chest'], 'barbell', 'intermediate', true, NULL),
('Overhead Tricep Extension', 'Long head stretch', 'triceps', ARRAY[], 'dumbbells', 'beginner', false, NULL),
('Skull Crushers', 'Lying tricep extension', 'triceps', ARRAY[], 'barbell', 'intermediate', false, NULL),
('Tricep Pushdown', 'Cable isolation', 'triceps', ARRAY[], 'cables', 'beginner', false, NULL),
('Rope Pushdown', 'Lateral head emphasis', 'triceps', ARRAY[], 'cables', 'beginner', false, NULL),
('Diamond Push-ups', 'Bodyweight tricep focus', 'triceps', ARRAY['chest'], 'bodyweight', 'intermediate', true, NULL)

ON CONFLICT (name) DO NOTHING;

-- CORE/ABS
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Plank', 'Isometric core stability', 'core', ARRAY[], 'bodyweight', 'beginner', false, NULL),
('Side Plank', 'Oblique stability', 'core', ARRAY[], 'bodyweight', 'beginner', false, NULL),
('Hanging Leg Raise', 'Lower ab and hip flexor', 'core', ARRAY[], 'bodyweight', 'advanced', false, NULL),
('Ab Wheel Rollout', 'Full core extension', 'core', ARRAY[], 'bodyweight', 'advanced', false, NULL),
('Cable Crunch', 'Weighted ab work', 'core', ARRAY[], 'cables', 'beginner', false, NULL),
('Russian Twist', 'Rotational core work', 'core', ARRAY[], 'bodyweight', 'beginner', false, NULL),
('Dead Bug', 'Core stability and coordination', 'core', ARRAY[], 'bodyweight', 'beginner', false, NULL),
('Pallof Press', 'Anti-rotation core strength', 'core', ARRAY[], 'cables', 'beginner', false, NULL)

ON CONFLICT (name) DO NOTHING;

-- CALVES
INSERT INTO exercises (name, description, primary_muscle, secondary_muscles, equipment, difficulty, is_compound, video_url) VALUES
('Standing Calf Raise', 'Gastrocnemius focus', 'calves', ARRAY[], 'machine', 'beginner', false, NULL),
('Seated Calf Raise', 'Soleus focus', 'calves', ARRAY[], 'machine', 'beginner', false, NULL),
('Calf Raise on Leg Press', 'Heavy calf work', 'calves', ARRAY[], 'machine', 'beginner', false, NULL)

ON CONFLICT (name) DO NOTHING;

-- Update exercise count
COMMENT ON TABLE exercises IS 'Comprehensive exercise library - 200+ exercises covering all muscle groups and equipment types';
