# Product Requirements Document (PRD): ESP32 OLED OTA, Web Updater & Firebase RTDB System

## 1. Project Overview
The **ESP32 OLED OTA, Web Updater & Firebase RTDB** project is an advanced embedded IoT firmware solution designed for ESP32 microcontrollers. It integrates an SSD1306 0.96-inch OLED display for real-time status visualization, an Over-The-Air (`ArduinoOTA`) service, a built-in Web Server (`WebServer`) providing a responsive Web Browser interface for monitoring and wireless firmware updates, and automated **Firebase Realtime Database (RTDB)** integration for telemetry and boot event logging.

---

## 2. Objectives & Scope
- **Visual Feedback:** Display startup initialization, connection status, IP address, NTP time sync, Firebase status, and OTA/Web update progress on an I2C OLED display.
- **Robust Connectivity:** Connect to local Wi-Fi networks in Station mode (`WIFI_STA`) with built-in retry logic and timeout handling.
- **Web-Based Dashboard & Updater:** Host a lightweight web server on port 80 presenting system specifications, memory usage, and an intuitive file upload form for browser-based firmware flashing.
- **Remote Maintainability:** Enable wireless firmware updates via `ArduinoOTA` or Web browser, eliminating the need for physical USB cable reconnection.
- **Cloud Telemetry (Firebase RTDB):** Automatically sync NTP time, push device boot events (`/bootEvents`), and update device status (`/devices/{mac}/lastBoot`) securely to Firebase RTDB.
- **Automated Deployment:** Provide local build and upload automation tools (`arduino-cli` and helper scripts).

---

## 3. Hardware Requirements & Pinout
- **Microcontroller:** ESP32 Dev Module (Wemos Lolin32 compatible)
- **Display:** 0.96" I2C SSD1306 OLED Display (Resolution: 128x64, I2C Address: `0x3C`)
- **Wiring / Pin Mapping (I2C):**
  - `SDA` -> GPIO 5
  - `SCL` -> GPIO 4
  - `VCC` -> 3.3V / 5V
  - `GND` -> Ground

---

## 4. Software Architecture & Dependencies
- **Core Platform:** Arduino IDE / `arduino-cli` with ESP32 Board Core (Espressif Systems `esp32:esp32`, v3.3.12)
- **Required Libraries:**
  - `Wire` (I2C communication)
  - `Adafruit GFX Library` & `Adafruit SSD1306` (OLED driver & graphics)
  - `WiFi` & `ESPmDNS` (Network stack and mDNS responder)
  - `ArduinoOTA` (Over-The-Air firmware updates)
  - `WebServer` & `Update` (HTTP server and flash update engine)
  - `Firebase_ESP_Client` (Mobizt Firebase Arduino Client for REST API / RTDB operations & `FirebaseJson`)

---

## 5. Functional Specifications & User Flow

### 5.1 Initialization & Main Loop (`setup` & `loop`)
1. **OLED Startup:** Initializes I2C bus on GPIO 5 (SDA) and GPIO 4 (SCL) and shows initialization status.
2. **Wi-Fi Connection:** Connects to configured SSID. Retries automatically upon timeout.
3. **Services Start:** Initializes NTP time sync (`pool.ntp.org`), Firebase client (`Firebase_ESP_Client`), and HTTP `WebServer` on port 80.
4. **OLED Status Display:** Continuously cycles status showing Hostname, local IP address, WiFi RSSI signal strength, Firebase sync status, time, and device Uptime.

### 5.2 Web Dashboard & System Info (`handleRoot`)
When accessing `http://<ESP32_IP>/` via web browser, the user is presented with a dashboard containing:
- **System Info Card:**
  - Chip Model & Revision (`ESP32-D0WD`, etc.)
  - CPU Frequency (240 MHz)
  - MAC Address (`WiFi.macAddress()`)
  - Free & Total Heap Memory (`ESP.getFreeHeap() / ESP.getHeapSize()`)
  - Flash Size (`ESP.getFlashChipSize()`)
  - Sketch Size (`ESP.getSketchSize()`)
  - Device Uptime
- **Firmware Update Card:**
  - File picker for compiled `.bin` firmware files.
  - Submit button to initiate wireless Web OTA flashing.

### 5.3 Firebase RTDB Telemetry & Boot Logging
1. **Time Synchronization:** Acquires accurate UTC time via NTP (`pool.ntp.org`) before pushing data to ensure valid ISO timestamps.
2. **URL Sanitization:** Strips protocol prefixes (`https://`, `http://`) and trailing slashes from `RTDB_URL` as required by `Firebase_ESP_Client`.
3. **Boot Event Push:** Pushes device boot metadata (MAC address, ISO date, GPS coordinates, boot time in ms, IP address) to `/bootEvents` (`Firebase.RTDB.pushJSON`).
4. **Device State Set:** Updates specific device record at `/devices/<mac>/lastBoot` (`Firebase.RTDB.setJSON`).

### 5.4 Web Browser & ArduinoOTA Update Progress
- **Progress Tracking:**
  - `onStart` / upload start: Switches OLED display to "WEB OTA START" or "OTA UPDATE START".
  - `onProgress` / upload progress: Displays real-time progress percentage and animated Progress Bar on OLED screen.
  - `onEnd` / success: Displays success message and triggers automatic device reboot.

---

## 6. Build & Deployment Workflow
- **CLI Compilation Tool:** `arduino-cli`
- **Target Board FQBN:** `esp32:esp32:esp32` (ESP32 Dev Module)
- **Serial Port:** `/dev/ttyUSB0`
- **Automation Script:** `./build_and_upload.sh`

---

## 7. Future Enhancements
- Password authentication for web server and OTA updates.
- WiFi Manager / Captive Portal for dynamic SSID configuration.
- Advanced telemetry logging and MQTT/Sensor integration.
