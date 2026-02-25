#!/bin/bash
set -e

echo "Running database migrations..."

if [ -z "$DATABASE_URL" ]; then
    echo "Error: DATABASE_URL not set"
    exit 1
fi

# Ensure schema_migrations table exists
psql "$DATABASE_URL" -c "
CREATE TABLE IF NOT EXISTS schema_migrations (
    version INTEGER PRIMARY KEY,
    applied_at TIMESTAMP DEFAULT NOW()
);
" 2>/dev/null || true

# Apply each migration in order
for file in migrations/*.sql; do
    if [ ! -f "$file" ]; then
        echo "No migration files found"
        exit 0
    fi

    # Extract version from filename (e.g., 001_create_users.sql -> 1)
    version=$(basename "$file" | grep -o '^[0-9]*' | sed 's/^0*//')
    if [ -z "$version" ]; then
        echo "Skipping $file (no version number)"
        continue
    fi

    # Check if already applied
    applied=$(psql "$DATABASE_URL" -t -c "SELECT COUNT(*) FROM schema_migrations WHERE version = $version;" 2>/dev/null | tr -d ' ')
    if [ "$applied" = "1" ]; then
        echo "  Skip: $file (already applied)"
        continue
    fi

    echo "  Applying: $file ..."
    psql "$DATABASE_URL" -f "$file"
    psql "$DATABASE_URL" -c "INSERT INTO schema_migrations (version) VALUES ($version) ON CONFLICT DO NOTHING;"
    echo "  Done: $file"
done

echo "Migrations complete!"
