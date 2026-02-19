-- Exercises table
CREATE TABLE exercises (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(100) NOT NULL,
    muscle_group VARCHAR(20) NOT NULL CHECK (muscle_group IN (
        'chest', 'back', 'shoulders', 'biceps', 'triceps',
        'quadriceps', 'hamstrings', 'glutes', 'calves', 'abs',
        'forearms', 'full_body', 'cardio', 'other'
    )),
    equipment VARCHAR(20) NOT NULL CHECK (equipment IN (
        'barbell', 'dumbbell', 'machine', 'cable', 'bodyweight',
        'kettlebell', 'band', 'other'
    )),
    is_custom BOOLEAN NOT NULL DEFAULT false,
    created_by UUID REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Unique constraint: built-in exercises have unique names, custom exercises are unique per user
CREATE UNIQUE INDEX idx_exercises_builtin_name ON exercises(name) WHERE is_custom = false;
CREATE UNIQUE INDEX idx_exercises_custom_name ON exercises(created_by, name) WHERE is_custom = true;

CREATE INDEX idx_exercises_muscle_group ON exercises(muscle_group);
CREATE INDEX idx_exercises_equipment ON exercises(equipment);
CREATE INDEX idx_exercises_created_by ON exercises(created_by) WHERE is_custom = true;

CREATE TRIGGER update_exercises_updated_at BEFORE UPDATE ON exercises
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Record migration
INSERT INTO schema_migrations (version) VALUES (3);
