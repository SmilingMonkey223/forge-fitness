-- Weight tracking log table
CREATE TABLE weight_logs (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    weight_kg DECIMAL(5,2) NOT NULL CHECK (weight_kg > 0 AND weight_kg < 500),
    logged_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_weight_logs_user_date ON weight_logs(user_id, logged_at DESC);

-- Record migration
INSERT INTO schema_migrations (version) VALUES (8);
