-- Nutrition log table
CREATE TABLE nutrition_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    logged_at TIMESTAMP NOT NULL,
    meal_type VARCHAR(20) CHECK (meal_type IN ('breakfast', 'lunch', 'dinner', 'snack')),
    food_name VARCHAR(200) NOT NULL,
    brand VARCHAR(100),
    serving_size DECIMAL(8,2) NOT NULL CHECK (serving_size > 0),
    serving_unit VARCHAR(20) NOT NULL,
    quantity DECIMAL(6,2) NOT NULL DEFAULT 1.0 CHECK (quantity > 0),
    calories DECIMAL(7,2) NOT NULL CHECK (calories >= 0),
    protein_g DECIMAL(6,2) NOT NULL CHECK (protein_g >= 0),
    carbs_g DECIMAL(6,2) NOT NULL CHECK (carbs_g >= 0),
    fat_g DECIMAL(6,2) NOT NULL CHECK (fat_g >= 0),
    fiber_g DECIMAL(6,2) CHECK (fiber_g >= 0),
    sugar_g DECIMAL(6,2) CHECK (sugar_g >= 0),
    sodium_mg DECIMAL(7,2) CHECK (sodium_mg >= 0),
    is_custom BOOLEAN NOT NULL DEFAULT false,
    source VARCHAR(20) NOT NULL DEFAULT 'manual' CHECK (source IN ('manual', 'database', 'barcode', 'ai')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    deleted_at TIMESTAMP
);

CREATE INDEX idx_nutrition_log_user_id_logged_at ON nutrition_log(user_id, logged_at DESC) WHERE deleted_at IS NULL;
CREATE INDEX idx_nutrition_log_user_id_date ON nutrition_log(user_id, DATE(logged_at)) WHERE deleted_at IS NULL;

CREATE TRIGGER update_nutrition_log_updated_at BEFORE UPDATE ON nutrition_log
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Custom foods table (user-created food entries)
CREATE TABLE custom_foods (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(200) NOT NULL,
    brand VARCHAR(100),
    serving_size DECIMAL(8,2) NOT NULL CHECK (serving_size > 0),
    serving_unit VARCHAR(20) NOT NULL,
    calories DECIMAL(7,2) NOT NULL CHECK (calories >= 0),
    protein_g DECIMAL(6,2) NOT NULL CHECK (protein_g >= 0),
    carbs_g DECIMAL(6,2) NOT NULL CHECK (carbs_g >= 0),
    fat_g DECIMAL(6,2) NOT NULL CHECK (fat_g >= 0),
    fiber_g DECIMAL(6,2) CHECK (fiber_g >= 0),
    sugar_g DECIMAL(6,2) CHECK (sugar_g >= 0),
    sodium_mg DECIMAL(7,2) CHECK (sodium_mg >= 0),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_custom_foods_user_id ON custom_foods(user_id);
CREATE INDEX idx_custom_foods_name ON custom_foods(user_id, name);

CREATE TRIGGER update_custom_foods_updated_at BEFORE UPDATE ON custom_foods
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- USDA food cache table
CREATE TABLE usda_food_cache (
    id SERIAL PRIMARY KEY,
    fdc_id INTEGER NOT NULL UNIQUE,
    name VARCHAR(300) NOT NULL,
    brand VARCHAR(100),
    serving_size DECIMAL(8,2),
    serving_unit VARCHAR(20),
    calories DECIMAL(7,2),
    protein_g DECIMAL(6,2),
    carbs_g DECIMAL(6,2),
    fat_g DECIMAL(6,2),
    fiber_g DECIMAL(6,2),
    sugar_g DECIMAL(6,2),
    sodium_mg DECIMAL(7,2),
    data JSONB,
    cached_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL
);

CREATE INDEX idx_usda_food_cache_name ON usda_food_cache USING gin(to_tsvector('english', name));
CREATE INDEX idx_usda_food_cache_expires_at ON usda_food_cache(expires_at);

-- Record migration
INSERT INTO schema_migrations (version) VALUES (6);
