#!/usr/bin/env bash
set -euo pipefail

# Deploy the ESP32 RTDB Monitor to Vercel and upload environment variables.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

# Pick the env file to upload (prefer rtdb.env, then .env, then .env.local)
ENV_FILE=""
for candidate in rtdb.env .env .env.local; do
  if [ -f "$candidate" ]; then
    ENV_FILE="$candidate"
    break
  fi
done

if [ -z "$ENV_FILE" ]; then
  echo "Error: no env file found (rtdb.env / .env / .env.local)."
  exit 1
fi

echo "Using env file: $ENV_FILE"

# Ensure Vercel CLI is available
if ! command -v vercel >/dev/null 2>&1; then
  VERCEL="npx --yes vercel"
else
  VERCEL="vercel"
fi

# Link project if not already linked
if [ ! -f ".vercel/project.json" ]; then
  echo "Linking project to Vercel..."
  $VERCEL link --yes
fi

# Upload each variable to Vercel (production, preview, development)
echo "Uploading environment variables from $ENV_FILE..."
while IFS= read -r line || [ -n "$line" ]; do
  # Trim leading/trailing whitespace
  line="$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
  # Skip comments and empty lines
  [[ -z "$line" || "$line" =~ ^# ]] && continue
  # Split on first '='
  key="${line%%=*}"
  value="${line#*=}"
  # Trim key
  key="$(echo "$key" | xargs)"
  # Remove surrounding quotes from value
  value="${value%\"}"; value="${value#\"}"
  value="${value%\'}"; value="${value#\'}"
  # Trim value
  value="$(echo "$value" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"

  # Skip PORT (Vercel manages it) and empty values
  [ "$key" = "PORT" ] && continue
  [ -z "$key" ] && continue
  [ -z "$value" ] && continue

  echo "  -> $key"
  $VERCEL env add "$key" \
    --environment production,preview,development \
    --value "$value" \
    --force --yes >/dev/null 2>&1 || {
      echo "     (add failed, trying update...)"
      $VERCEL env update "$key" \
        --environment production \
        --value "$value" --yes >/dev/null 2>&1 || true
    }
done < "$ENV_FILE"

echo "Environment variables uploaded."

# Deploy to production
echo "Deploying to Vercel (production)..."
$VERCEL --prod --yes

echo "Done."