-- Workouts table
CREATE TABLE workouts (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(100),
    started_at TIMESTAMP NOT NULL,
    completed_at TIMESTAMP,
    duration_seconds INTEGER,
    notes TEXT CHECK (LENGTH(notes) <= 2000),
    status VARCHAR(20) NOT NULL DEFAULT 'in_progress' CHECK (status IN ('in_progress', 'completed', 'cancelled')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    deleted_at TIMESTAMP
);

CREATE INDEX idx_workouts_user_id_started_at ON workouts(user_id, started_at DESC) WHERE deleted_at IS NULL;
CREATE INDEX idx_workouts_status ON workouts(status) WHERE deleted_at IS NULL;

CREATE TRIGGER update_workouts_updated_at BEFORE UPDATE ON workouts
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Exercise sets table
CREATE TABLE exercise_sets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workout_id UUID NOT NULL REFERENCES workouts(id) ON DELETE CASCADE,
    exercise_id UUID NOT NULL REFERENCES exercises(id) ON DELETE RESTRICT,
    set_order INTEGER NOT NULL CHECK (set_order >= 1),
    exercise_order INTEGER NOT NULL CHECK (exercise_order >= 1),
    set_type VARCHAR(20) NOT NULL DEFAULT 'working' CHECK (set_type IN ('warmup', 'working', 'drop_set', 'failure')),
    reps INTEGER CHECK (reps >= 0 AND reps <= 999),
    weight_kg DECIMAL(6,2) CHECK (weight_kg >= 0 AND weight_kg <= 999.99),
    duration_seconds INTEGER CHECK (duration_seconds >= 0),
    rpe DECIMAL(3,1) CHECK (rpe >= 1.0 AND rpe <= 10.0 AND (rpe * 2)::INTEGER = (rpe * 2)),
    rest_seconds INTEGER CHECK (rest_seconds >= 0),
    is_pr BOOLEAN NOT NULL DEFAULT false,
    notes VARCHAR(500),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT check_reps_or_duration CHECK (
        (reps IS NOT NULL AND duration_seconds IS NULL) OR
        (reps IS NULL AND duration_seconds IS NOT NULL) OR
        (reps IS NOT NULL AND duration_seconds IS NOT NULL)
    )
);

CREATE INDEX idx_exercise_sets_workout_id ON exercise_sets(workout_id);
CREATE INDEX idx_exercise_sets_exercise_id ON exercise_sets(exercise_id);
CREATE INDEX idx_exercise_sets_workout_exercise ON exercise_sets(workout_id, exercise_order, set_order);

CREATE TRIGGER update_exercise_sets_updated_at BEFORE UPDATE ON exercise_sets
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Record migration
INSERT INTO schema_migrations (version) VALUES (4);
