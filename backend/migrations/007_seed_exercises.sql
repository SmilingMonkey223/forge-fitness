-- Seed 200+ exercises covering all major muscle groups and equipment types

-- CHEST EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Chest
('Barbell Bench Press', 'chest', 'barbell', false),
('Incline Barbell Bench Press', 'chest', 'barbell', false),
('Decline Barbell Bench Press', 'chest', 'barbell', false),
('Close Grip Bench Press', 'chest', 'barbell', false),
('Barbell Floor Press', 'chest', 'barbell', false),

-- Dumbbell Chest
('Dumbbell Bench Press', 'chest', 'dumbbell', false),
('Incline Dumbbell Press', 'chest', 'dumbbell', false),
('Decline Dumbbell Press', 'chest', 'dumbbell', false),
('Dumbbell Flyes', 'chest', 'dumbbell', false),
('Incline Dumbbell Flyes', 'chest', 'dumbbell', false),
('Dumbbell Pullover', 'chest', 'dumbbell', false),

-- Machine Chest
('Machine Chest Press', 'chest', 'machine', false),
('Incline Machine Press', 'chest', 'machine', false),
('Pec Deck Flye', 'chest', 'machine', false),
('Machine Chest Flye', 'chest', 'machine', false),

-- Cable Chest
('Cable Flyes', 'chest', 'cable', false),
('Low to High Cable Flyes', 'chest', 'cable', false),
('High to Low Cable Flyes', 'chest', 'cable', false),
('Cable Crossover', 'chest', 'cable', false),

-- Bodyweight Chest
('Push Ups', 'chest', 'bodyweight', false),
('Wide Grip Push Ups', 'chest', 'bodyweight', false),
('Diamond Push Ups', 'chest', 'bodyweight', false),
('Decline Push Ups', 'chest', 'bodyweight', false),
('Dips', 'chest', 'bodyweight', false);

-- BACK EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Back
('Barbell Row', 'back', 'barbell', false),
('Pendlay Row', 'back', 'barbell', false),
('T-Bar Row', 'back', 'barbell', false),
('Barbell Deadlift', 'back', 'barbell', false),
('Rack Pull', 'back', 'barbell', false),
('Romanian Deadlift', 'back', 'barbell', false),
('Barbell Shrug', 'back', 'barbell', false),

-- Dumbbell Back
('Dumbbell Row', 'back', 'dumbbell', false),
('One Arm Dumbbell Row', 'back', 'dumbbell', false),
('Incline Dumbbell Row', 'back', 'dumbbell', false),
('Dumbbell Deadlift', 'back', 'dumbbell', false),
('Dumbbell Shrug', 'back', 'dumbbell', false),
('Dumbbell Pullover', 'back', 'dumbbell', false),

-- Machine Back
('Lat Pulldown', 'back', 'machine', false),
('Wide Grip Lat Pulldown', 'back', 'machine', false),
('Close Grip Lat Pulldown', 'back', 'machine', false),
('Neutral Grip Lat Pulldown', 'back', 'machine', false),
('Seated Cable Row', 'back', 'machine', false),
('T-Bar Row Machine', 'back', 'machine', false),
('Chest Supported Row', 'back', 'machine', false),
('Machine Row', 'back', 'machine', false),
('Assisted Pull Up', 'back', 'machine', false),

-- Cable Back
('Cable Row', 'back', 'cable', false),
('Single Arm Cable Row', 'back', 'cable', false),
('Face Pull', 'back', 'cable', false),
('Straight Arm Pulldown', 'back', 'cable', false),

-- Bodyweight Back
('Pull Ups', 'back', 'bodyweight', false),
('Chin Ups', 'back', 'bodyweight', false),
('Wide Grip Pull Ups', 'back', 'bodyweight', false),
('Neutral Grip Pull Ups', 'back', 'bodyweight', false),
('Inverted Row', 'back', 'bodyweight', false);

-- SHOULDER EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Shoulders
('Barbell Overhead Press', 'shoulders', 'barbell', false),
('Seated Barbell Press', 'shoulders', 'barbell', false),
('Behind the Neck Press', 'shoulders', 'barbell', false),
('Barbell Front Raise', 'shoulders', 'barbell', false),
('Barbell Upright Row', 'shoulders', 'barbell', false),

-- Dumbbell Shoulders
('Dumbbell Shoulder Press', 'shoulders', 'dumbbell', false),
('Seated Dumbbell Press', 'shoulders', 'dumbbell', false),
('Arnold Press', 'shoulders', 'dumbbell', false),
('Dumbbell Lateral Raise', 'shoulders', 'dumbbell', false),
('Dumbbell Front Raise', 'shoulders', 'dumbbell', false),
('Dumbbell Rear Delt Flye', 'shoulders', 'dumbbell', false),
('Dumbbell Upright Row', 'shoulders', 'dumbbell', false),

-- Machine Shoulders
('Machine Shoulder Press', 'shoulders', 'machine', false),
('Machine Lateral Raise', 'shoulders', 'machine', false),
('Machine Reverse Flye', 'shoulders', 'machine', false),

-- Cable Shoulders
('Cable Lateral Raise', 'shoulders', 'cable', false),
('Cable Front Raise', 'shoulders', 'cable', false),
('Cable Rear Delt Flye', 'shoulders', 'cable', false),
('Cable Face Pull', 'shoulders', 'cable', false),

-- Bodyweight/Other Shoulders
('Pike Push Ups', 'shoulders', 'bodyweight', false),
('Handstand Push Ups', 'shoulders', 'bodyweight', false);

-- BICEPS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Biceps
('Barbell Curl', 'biceps', 'barbell', false),
('EZ Bar Curl', 'biceps', 'barbell', false),
('Preacher Curl', 'biceps', 'barbell', false),
('Close Grip Barbell Curl', 'biceps', 'barbell', false),
('Reverse Grip Barbell Curl', 'biceps', 'barbell', false),

-- Dumbbell Biceps
('Dumbbell Curl', 'biceps', 'dumbbell', false),
('Alternating Dumbbell Curl', 'biceps', 'dumbbell', false),
('Hammer Curl', 'biceps', 'dumbbell', false),
('Incline Dumbbell Curl', 'biceps', 'dumbbell', false),
('Concentration Curl', 'biceps', 'dumbbell', false),
('Dumbbell Preacher Curl', 'biceps', 'dumbbell', false),

-- Machine/Cable Biceps
('Cable Curl', 'biceps', 'cable', false),
('High Cable Curl', 'biceps', 'cable', false),
('Low Cable Curl', 'biceps', 'cable', false),
('Machine Preacher Curl', 'biceps', 'machine', false),

-- Bodyweight Biceps
('Chin Ups', 'biceps', 'bodyweight', false);

-- TRICEPS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Triceps
('Close Grip Bench Press', 'triceps', 'barbell', false),
('Barbell Skull Crusher', 'triceps', 'barbell', false),
('JM Press', 'triceps', 'barbell', false),

-- Dumbbell Triceps
('Dumbbell Overhead Extension', 'triceps', 'dumbbell', false),
('Dumbbell Skull Crusher', 'triceps', 'dumbbell', false),
('Dumbbell Kickback', 'triceps', 'dumbbell', false),
('Two Arm Dumbbell Extension', 'triceps', 'dumbbell', false),

-- Machine/Cable Triceps
('Cable Pushdown', 'triceps', 'cable', false),
('Rope Pushdown', 'triceps', 'cable', false),
('Overhead Cable Extension', 'triceps', 'cable', false),
('Machine Dip', 'triceps', 'machine', false),

-- Bodyweight Triceps
('Tricep Dips', 'triceps', 'bodyweight', false),
('Bench Dips', 'triceps', 'bodyweight', false),
('Diamond Push Ups', 'triceps', 'bodyweight', false);

-- QUADRICEPS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Quads
('Barbell Back Squat', 'quadriceps', 'barbell', false),
('Barbell Front Squat', 'quadriceps', 'barbell', false),
('High Bar Squat', 'quadriceps', 'barbell', false),
('Low Bar Squat', 'quadriceps', 'barbell', false),
('Barbell Lunge', 'quadriceps', 'barbell', false),
('Barbell Step Up', 'quadriceps', 'barbell', false),
('Barbell Bulgarian Split Squat', 'quadriceps', 'barbell', false),

-- Dumbbell Quads
('Goblet Squat', 'quadriceps', 'dumbbell', false),
('Dumbbell Lunge', 'quadriceps', 'dumbbell', false),
('Dumbbell Step Up', 'quadriceps', 'dumbbell', false),
('Dumbbell Bulgarian Split Squat', 'quadriceps', 'dumbbell', false),
('Dumbbell Squat', 'quadriceps', 'dumbbell', false),

-- Machine Quads
('Leg Press', 'quadriceps', 'machine', false),
('Hack Squat', 'quadriceps', 'machine', false),
('Leg Extension', 'quadriceps', 'machine', false),
('Smith Machine Squat', 'quadriceps', 'machine', false),

-- Bodyweight Quads
('Bodyweight Squat', 'quadriceps', 'bodyweight', false),
('Jump Squat', 'quadriceps', 'bodyweight', false),
('Pistol Squat', 'quadriceps', 'bodyweight', false),
('Walking Lunge', 'quadriceps', 'bodyweight', false);

-- HAMSTRINGS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Hamstrings
('Romanian Deadlift', 'hamstrings', 'barbell', false),
('Stiff Leg Deadlift', 'hamstrings', 'barbell', false),
('Good Morning', 'hamstrings', 'barbell', false),

-- Dumbbell Hamstrings
('Dumbbell RDL', 'hamstrings', 'dumbbell', false),
('Dumbbell Stiff Leg Deadlift', 'hamstrings', 'dumbbell', false),
('Dumbbell Good Morning', 'hamstrings', 'dumbbell', false),

-- Machine Hamstrings
('Lying Leg Curl', 'hamstrings', 'machine', false),
('Seated Leg Curl', 'hamstrings', 'machine', false),
('Nordic Curl', 'hamstrings', 'machine', false),

-- Bodyweight Hamstrings
('Glute Ham Raise', 'hamstrings', 'bodyweight', false),
('Single Leg RDL', 'hamstrings', 'bodyweight', false);

-- GLUTES EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell Glutes
('Hip Thrust', 'glutes', 'barbell', false),
('Barbell Glute Bridge', 'glutes', 'barbell', false),

-- Dumbbell Glutes
('Dumbbell Hip Thrust', 'glutes', 'dumbbell', false),
('Dumbbell Glute Bridge', 'glutes', 'dumbbell', false),

-- Machine Glutes
('Hip Abduction Machine', 'glutes', 'machine', false),
('Cable Pull Through', 'glutes', 'cable', false),
('Machine Glute Kickback', 'glutes', 'machine', false),

-- Bodyweight/Other Glutes
('Glute Bridge', 'glutes', 'bodyweight', false),
('Single Leg Glute Bridge', 'glutes', 'bodyweight', false),
('Donkey Kick', 'glutes', 'bodyweight', false),
('Fire Hydrant', 'glutes', 'bodyweight', false),
('Kettlebell Swing', 'glutes', 'kettlebell', false);

-- CALVES EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Machine Calves
('Standing Calf Raise', 'calves', 'machine', false),
('Seated Calf Raise', 'calves', 'machine', false),
('Leg Press Calf Raise', 'calves', 'machine', false),

-- Dumbbell/Bodyweight Calves
('Dumbbell Calf Raise', 'calves', 'dumbbell', false),
('Single Leg Calf Raise', 'calves', 'bodyweight', false);

-- ABS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Bodyweight Abs
('Crunch', 'abs', 'bodyweight', false),
('Bicycle Crunch', 'abs', 'bodyweight', false),
('Russian Twist', 'abs', 'bodyweight', false),
('Leg Raise', 'abs', 'bodyweight', false),
('Hanging Leg Raise', 'abs', 'bodyweight', false),
('Plank', 'abs', 'bodyweight', false),
('Side Plank', 'abs', 'bodyweight', false),
('Mountain Climber', 'abs', 'bodyweight', false),
('Flutter Kick', 'abs', 'bodyweight', false),
('Sit Up', 'abs', 'bodyweight', false),
('V-Up', 'abs', 'bodyweight', false),
('Dead Bug', 'abs', 'bodyweight', false),
('Hollow Body Hold', 'abs', 'bodyweight', false),

-- Cable/Machine Abs
('Cable Crunch', 'abs', 'cable', false),
('Cable Wood Chop', 'abs', 'cable', false),
('Ab Wheel Rollout', 'abs', 'other', false),

-- Weighted Abs
('Weighted Crunch', 'abs', 'dumbbell', false),
('Dumbbell Side Bend', 'abs', 'dumbbell', false);

-- FOREARMS EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
-- Barbell/Dumbbell Forearms
('Wrist Curl', 'forearms', 'barbell', false),
('Reverse Wrist Curl', 'forearms', 'barbell', false),
('Dumbbell Wrist Curl', 'forearms', 'dumbbell', false),
('Farmers Walk', 'forearms', 'dumbbell', false),

-- Other Forearms
('Plate Pinch', 'forearms', 'other', false),
('Dead Hang', 'forearms', 'bodyweight', false);

-- CARDIO EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
('Running', 'cardio', 'other', false),
('Treadmill', 'cardio', 'machine', false),
('Cycling', 'cardio', 'machine', false),
('Rowing Machine', 'cardio', 'machine', false),
('Elliptical', 'cardio', 'machine', false),
('Stair Climber', 'cardio', 'machine', false),
('Jump Rope', 'cardio', 'other', false),
('Burpees', 'cardio', 'bodyweight', false),
('Box Jumps', 'cardio', 'bodyweight', false),
('Swimming', 'cardio', 'other', false);

-- FULL BODY EXERCISES
INSERT INTO exercises (name, muscle_group, equipment, is_custom) VALUES
('Barbell Clean', 'full_body', 'barbell', false),
('Power Clean', 'full_body', 'barbell', false),
('Clean and Jerk', 'full_body', 'barbell', false),
('Snatch', 'full_body', 'barbell', false),
('Thruster', 'full_body', 'barbell', false),
('Turkish Get Up', 'full_body', 'kettlebell', false),
('Kettlebell Swing', 'full_body', 'kettlebell', false);

-- Record migration
INSERT INTO schema_migrations (version) VALUES (7);
