#!/usr/bin/env bash
set -e
# Generate config.h from .env
ENV_FILE="${1:-.env}"
OUT="config.h"

get() {
  local key="$1"
  local val
  val=$(grep -E "^${key}=" "$ENV_FILE" | head -n1 | cut -d= -f2-)
  printf '%s' "$val"
}

WIFI_SSID=$(get WIFI_SSID)
WIFI_PASSWORD=$(get WIFI_PASSWORD)
API_KEY=$(get API_KEY)
# fallback to FIREBASE_API_KEY / FIREBASE_DATABASE_URL
[ -z "$API_KEY" ] && API_KEY=$(get FIREBASE_API_KEY)
RTDB_URL=$(get RTDB_URL)
[ -z "$RTDB_URL" ] && RTDB_URL=$(get FIREBASE_DATABASE_URL)

GPS_LAT=$(get GPS_LAT); [ -z "$GPS_LAT" ] && GPS_LAT="0.000000"
GPS_LON=$(get GPS_LON); [ -z "$GPS_LON" ] && GPS_LON="0.000000"

cat > "$OUT" <<HEADER
#pragma once

// Auto-generated from $ENV_FILE by gen_config.sh - do not edit manually.
#define WIFI_SSID "$WIFI_SSID"
#define WIFI_PASSWORD "$WIFI_PASSWORD"

#define API_KEY "$API_KEY"
#define RTDB_URL "$RTDB_URL"

#define GPS_LAT "$GPS_LAT"
#define GPS_LON "$GPS_LON"
HEADER
echo "Generated $OUT"
