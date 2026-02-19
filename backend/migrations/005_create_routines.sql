-- Routine templates table
CREATE TABLE routines (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    deleted_at TIMESTAMP
);

CREATE INDEX idx_routines_user_id ON routines(user_id) WHERE deleted_at IS NULL;

CREATE TRIGGER update_routines_updated_at BEFORE UPDATE ON routines
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Routine exercises table
CREATE TABLE routine_exercises (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    routine_id UUID NOT NULL REFERENCES routines(id) ON DELETE CASCADE,
    exercise_id UUID NOT NULL REFERENCES exercises(id) ON DELETE RESTRICT,
    exercise_order INTEGER NOT NULL CHECK (exercise_order >= 1),
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_routine_exercises_routine_id ON routine_exercises(routine_id);
CREATE UNIQUE INDEX idx_routine_exercises_order ON routine_exercises(routine_id, exercise_order);

-- Routine sets (template for target sets/reps)
CREATE TABLE routine_sets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    routine_exercise_id UUID NOT NULL REFERENCES routine_exercises(id) ON DELETE CASCADE,
    set_order INTEGER NOT NULL CHECK (set_order >= 1),
    target_reps_min INTEGER CHECK (target_reps_min >= 0),
    target_reps_max INTEGER CHECK (target_reps_max >= target_reps_min),
    target_duration_seconds INTEGER CHECK (target_duration_seconds >= 0),
    set_type VARCHAR(20) NOT NULL DEFAULT 'working' CHECK (set_type IN ('warmup', 'working', 'drop_set', 'failure')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_routine_sets_routine_exercise_id ON routine_sets(routine_exercise_id);
CREATE UNIQUE INDEX idx_routine_sets_order ON routine_sets(routine_exercise_id, set_order);

-- Record migration
INSERT INTO schema_migrations (version) VALUES (5);
