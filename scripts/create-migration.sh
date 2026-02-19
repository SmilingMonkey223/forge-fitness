#!/bin/bash

# Script to create a new database migration

if [ -z "$1" ]; then
    echo "Usage: ./scripts/create-migration.sh <migration_name>"
    echo "Example: ./scripts/create-migration.sh add_user_preferences"
    exit 1
fi

MIGRATION_NAME=$1
MIGRATIONS_DIR="backend/migrations"

# Find the next migration number
LAST_MIGRATION=$(ls -1 $MIGRATIONS_DIR/*.sql 2>/dev/null | tail -n 1)
if [ -z "$LAST_MIGRATION" ]; then
    NEXT_NUM=1
else
    LAST_NUM=$(basename "$LAST_MIGRATION" | cut -d'_' -f1)
    NEXT_NUM=$((LAST_NUM + 1))
fi

# Format number with leading zeros
FORMATTED_NUM=$(printf "%03d" $NEXT_NUM)

# Create migration file
MIGRATION_FILE="$MIGRATIONS_DIR/${FORMATTED_NUM}_${MIGRATION_NAME}.sql"

cat > "$MIGRATION_FILE" << EOF
-- Migration: $MIGRATION_NAME
-- Created: $(date +"%Y-%m-%d %H:%M:%S")

-- Your migration SQL goes here


-- Record migration
INSERT INTO schema_migrations (version) VALUES ($NEXT_NUM);
EOF

echo "Created migration: $MIGRATION_FILE"
echo "Edit the file to add your migration SQL, then run: make migrate"
