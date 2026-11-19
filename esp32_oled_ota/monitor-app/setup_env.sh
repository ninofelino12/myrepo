#!/bin/bash

echo "Setting up environment variables for ESP32 RTDB Monitor..."

# Path to root .env or config.h
ROOT_ENV="../.env"
ROOT_CONFIG="../config.h"

# Default values
PORT=3000
API_KEY=""
RTDB_URL=""

# Try reading from root .env if exists
if [ -f "$ROOT_ENV" ]; then
    echo "Reading from root .env..."
    while IFS='=' read -r key value; do
        [[ "$key" =~ ^#.*$ ]] && continue
        [[ -z "$key" ]] && continue
        
        key=$(echo "$key" | xargs)
        value=$(echo "$value" | xargs)
        
        if [ "$key" = "API_KEY" ] || [ "$key" = "FIREBASE_API_KEY" ]; then
            API_KEY="$value"
        elif [ "$key" = "RTDB_URL" ] || [ "$key" = "FIREBASE_DATABASE_URL" ]; then
            RTDB_URL="$value"
        fi
    done < "$ROOT_ENV"
fi

# Fallback to config.h if API_KEY or RTDB_URL is empty
if [ -z "$API_KEY" ] && [ -f "$ROOT_CONFIG" ]; then
    API_KEY=$(grep "#define API_KEY" "$ROOT_CONFIG" | cut -d'"' -f2)
fi

if [ -z "$RTDB_URL" ] && [ -f "$ROOT_CONFIG" ]; then
    RTDB_URL=$(grep "#define RTDB_URL" "$ROOT_CONFIG" | cut -d'"' -f2)
fi

# Fallback defaults if still empty
API_KEY="${API_KEY:-AIzaSyAvWCSjfvz4R5kduVDtTiPiPrY-nUJki6o}"
RTDB_URL="${RTDB_URL:-https://bizflow-dashboard-vlo72-default-rtdb.asia-southeast1.firebasedatabase.app}"

# Write to rtdb.env
cat << EOF > rtdb.env
PORT=3000
API_KEY=$API_KEY
RTDB_URL=$RTDB_URL
FIREBASE_PROJECT_ID=bizflow-dashboard-vlo72
EOF

echo "Successfully generated monitor-app/rtdb.env:"
cat rtdb.env

echo ""
read -p "Do you want to link project and push environment variables to Vercel? (y/N) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if command -v npx &> /dev/null; then
        echo "Linking project to Vercel..."
        npx --yes vercel link --yes
        
        echo "Pushing RTDB_URL to Vercel..."
        echo "$RTDB_URL" | npx --yes vercel env add RTDB_URL production --yes
        
        echo "Pushing API_KEY to Vercel..."
        echo "$API_KEY" | npx --yes vercel env add API_KEY production --yes
        
        echo "Environment variables linked and pushed to Vercel successfully!"
    else
        echo "Error: npx not found. Please install Node.js."
    fi
else
    echo "Skipped Vercel env push."
fi
