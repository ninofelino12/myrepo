# Skill Guide: ESP32 IoT Development with Firebase RTDB, OLED, & Web OTA

## Overview
This skill guide captures expert patterns, configuration rules, and troubleshooting strategies for developing and maintaining ESP32 firmware utilizing:
- **Mobizt Firebase Client Library (`Firebase_ESP_Client`)** for Realtime Database (RTDB) operations.
- **SSD1306 OLED Display** (`Adafruit_SSD1306`) for real-time visual feedback.
- **ESP32 Web Server & ArduinoOTA** for wireless firmware flashing.
- **`arduino-cli`** for local automated builds and deployments.

---

## 1. Firebase RTDB Integration Best Practices (`Firebase_ESP_Client`)

### A. Database URL Formatting
- **Rule:** `Firebase_ESP_Client` expects `fbConfig.database_url` **without** protocol prefixes (`https://`, `http://`) and **without** trailing slashes (`/`).
- **Implementation Pattern:**
  ```String url = RTDB_URL;
  url.replace("https://", "");
  url.replace("http://", "");
  if (url.endsWith("/")) url.remove(url.length() - 1);
  fbConfig.database_url = url.c_str();
  ```

### B. API Key & Authentication
- Always configure `fbConfig.api_key = API_KEY;` even when running in test mode (`fbConfig.signer.test_mode = true`).
- Ensure Firebase Realtime Database Security Rules allow appropriate read/write access (e.g. public rules for testing or token/auth rules for production).

### C. Time Synchronization (NTP)
- SSL/TLS handshakes and token validations require accurate system time.
- Always synchronize time via NTP (`pool.ntp.org`, `time.nist.gov`) before attempting authenticated or secure REST API calls to Firebase.

---

## 2. Troubleshooting Common Firebase Errors

### HTTP 400 (Bad Request)
- **Cause:** Incorrect `database_url` format (e.g., keeping `https://` prefix) or malformed JSON payload.
- **Solution:** Sanitize `database_url` as shown above and verify `FirebaseJson` data structure. Log `fbdo.payload()` on failure to inspect server error details.

### HTTP 401 / 403 (Unauthorized / Permission Denied)
- **Cause:** Firebase RTDB Security Rules reject unauthenticated requests or missing `API_KEY`.
- **Solution:** Publish public rules (`.read: true`, `.write: true`) for test mode or configure proper auth credentials.

---

## 3. Build & Deployment Workflow (`arduino-cli`)
- Target Board FQBN: `esp32:esp32:esp32`
- Compilation command:
  ```bash
  arduino-cli compile --fqbn esp32:esp32:esp32 .
  ```
- Upload command:
  ```bash
  arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
  ```
