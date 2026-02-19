-- User profiles table
CREATE TABLE user_profiles (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL UNIQUE REFERENCES users(id) ON DELETE CASCADE,
    date_of_birth DATE NOT NULL,
    sex VARCHAR(10) NOT NULL CHECK (sex IN ('male', 'female', 'other')),
    height_cm DECIMAL(5,2) NOT NULL CHECK (height_cm BETWEEN 100 AND 250),
    weight_kg DECIMAL(5,2) NOT NULL CHECK (weight_kg BETWEEN 30 AND 300),
    activity_level VARCHAR(20) NOT NULL CHECK (activity_level IN ('sedentary', 'lightly_active', 'moderately_active', 'very_active', 'extremely_active')),
    fitness_goal VARCHAR(20) NOT NULL CHECK (fitness_goal IN ('lose_fat', 'maintain', 'build_muscle')),
    unit_preference VARCHAR(10) NOT NULL DEFAULT 'metric' CHECK (unit_preference IN ('metric', 'imperial')),
    tdee_calories DECIMAL(6,1),
    target_calories DECIMAL(6,1),
    target_protein_g DECIMAL(5,1),
    target_carbs_g DECIMAL(5,1),
    target_fat_g DECIMAL(5,1),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_user_profiles_user_id ON user_profiles(user_id);

CREATE TRIGGER update_user_profiles_updated_at BEFORE UPDATE ON user_profiles
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Record migration
INSERT INTO schema_migrations (version) VALUES (2);
