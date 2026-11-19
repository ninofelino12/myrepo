# ESP32 & ESP8266 IoT OTA, OLED Display & Firebase RTDB Monitoring Ecosystem

Repositori ini berisi ekosistem Internet of Things (IoT) lengkap berbasis mikrokontroler **ESP32** dan **ESP8266**, dilengkapi dengan layar **OLED SSD1306**, pembaruan firmware nirkabel (**OTA & Web Server Updater**), serta integrasi **Firebase Realtime Database (RTDB)** dan aplikasi web dashboard monitoring (**Node.js / Express**).
[![Kalender](https://img.shields.io/badge/📅_Buka_Kalender-Klik_Disini-blue)](https://monitor-ai64eh1m7-ninofelinos-projects.vercel.app/calendar)
---

## 📂 Struktur Repositori

```text
myrepo/
├── esp32_oled_ota/         # Firmware ESP32 utama (OLED, Web OTA, ArduinoOTA, Firebase RTDB)
│   ├── monitor-app/        # Dashboard monitoring web lokal (Express.js) khusus ESP32
│   ├── config.h            # Konfigurasi WiFi, Firebase RTDB, & NTP
│   ├── esp32_oled_ota.ino  # Kode sumber utama ESP32
│   ├── build_and_upload.sh # Skrip otomatis kompilasi dan upload via arduino-cli
│   ├── monitor.sh          # Skrip serial monitor otomatis
│   ├── prd.md              # Product Requirements Document (PRD)
│   └── panduan_pemakaian.md # Panduan pemakaian lengkap firmware ESP32
├── esp8266/                # Firmware alternatif untuk modul ESP8266
│   ├── config.h            # Konfigurasi ESP8266
│   ├── esp8266.ino         # Kode sumber ESP8266
│   └── deploy.sh           # Skrip deployment ESP8266
└── monitor-app/            # Aplikasi dashboard web pusat (Express.js & Tailwind CSS)
    ├── server.js           # Server backend Express & integrasi Firebase RTDB
    ├── views/              # EJS template (index, logs, calendar)
    └── vercel.json         # Konfigurasi deployment Vercel
```

---

## 🚀 Komponen Utama

### 1. ESP32 OLED OTA Firmware (`esp32_oled_ota/`)
Firmware mikrokontroler ESP32 Dev Module / Lolin32 yang menawarkan:
- **OLED Display (SSD1306 0.96"):** Menampilkan status inisialisasi, alamat IP, kekuatan sinyal WiFi (RSSI), waktu NTP, dan status sinkronisasi Firebase.
- **Web Server & Web OTA:** Antarmuka web lokal untuk memantau spesifikasi sistem (Heap, Flash, Uptime) dan melakukan *flash* firmware `.bin` secara nirkabel (`http://<IP_ESP32>/`).
- **ArduinoOTA:** Kemampuan update firmware nirkabel langsung melalui jaringan lokal.
- **Firebase RTDB Telemetry:** Otomatis menyinkronkan waktu via NTP dan mengirim log <i>boot event</i> serta status perangkat (`/bootEvents` & `/devices/<mac>/lastBoot`) ke Firebase Realtime Database.

### 2. ESP8266 Firmware (`esp8266/`)
Firmware pendukung untuk mikrokontroler ESP8266 dengan fitur koneksi WiFi dan fungsionalitas pendukung.

### 3. Monitor Web Application (`monitor-app/`)
Aplikasi web berbasis **Node.js (Express.js)** dan **Tailwind CSS** untuk memantau data log perangkat IoT secara <i>real-time</i> dari Firebase RTDB:
- **Live Boot Events Log:** Memantau riwayat *boot* perangkat IoT.
- **Device Status:** Memantau daftar perangkat terdaftar beserta informasi terakhirnya.
- **Siap Cloud Deployment:** Dikonfigurasi untuk deployment instan ke **Vercel**.

---

## 🛠️ Panduan Singkat Penggunaan

### A. Konfigurasi dan Upload Firmware ESP32
1. Masuk ke direktori `esp32_oled_ota`:
   ```bash
   cd esp32_oled_ota
   ```
2. Salin/sesuaikan file konfigurasi `config.h` (atur SSID WiFi, Password, dan Kredensial Firebase RTDB).
3. Kompilasi dan *upload* menggunakan skrip otomatis (`arduino-cli`):
   ```bash
   ./build_and_upload.sh
   ```
4. Buka serial monitor:
   ```bash
   ./monitor.sh
   ```
   *Catatan lengkap tersedia di [panduan_pemakaian.md](./esp32_oled_ota/panduan_pemakaian.md).*

### B. Menjalankan Dashboard Monitor Web Lokal
1. Masuk ke direktori `monitor-app`:
   ```bash
   cd monitor-app
   ```
2. Install dependensi Node.js:
   ```bash
   npm install
   ```
3. Konfigurasikan variabel lingkungan pada file `.env.local` atau `rtdb.env` dengan URL Firebase RTDB dan API Key Anda.
4. Jalankan server:
   ```bash
   npm start
   ```
5. Akses melalui browser di `http://localhost:3000`.

---

## 📖 Dokumentasi Terkait
- [PRD ESP32 OLED OTA](./esp32_oled_ota/prd.md)
- [Panduan Pemakaian ESP32](./esp32_oled_ota/panduan_pemakaian.md)
- [README Monitor App](./monitor-app/README.md)
