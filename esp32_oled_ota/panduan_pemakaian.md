# Panduan Pemakaian: ESP32 OLED OTA & Web Updater

Dokumen ini adalah panduan lengkap untuk merakit, mengompilasi, meng-upload, dan melakukan pembaruan firmware (OTA & Web Browser) pada perangkat **ESP32 Dev Module / Lolin32** dengan layar **OLED SSD1306**.

---

## 1. Skema Perangkat Keras & Pinout (I2C OLED)
Hubungkan modul OLED SSD1306 0.96" (I2C, Address `0x3C`) ke ESP32 dengan koneksi berikut:
- **VCC** -> 3.3V / 5V (sesuai modul OLED Anda)
- **GND** -> GND
- **SDA** -> GPIO 5
- **SCL** -> GPIO 4

---

## 2. Persiapan Lingkungan & Pustaka (Libraries)
Pastikan Anda telah memasang:
1. **Arduino CLI** atau **Arduino IDE**
2. **ESP32 Board Package** (Espressif Systems `esp32:esp32`, v3.3.12)
3. Pustaka Arduino berikut:
   - `Wire` (bawaan core ESP32)
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `Adafruit BusIO`
   - `WiFi`, `ESPmDNS`, `ArduinoOTA`, `WebServer`, `Update`, `FS`, `Hash` (bawaan core ESP32)

---

## 3. Kompilasi, Upload & Monitor (via USB)
Hubungkan ESP32 ke komputer menggunakan kabel USB ke port serial (misal: `/dev/ttyUSB0` atau `COM3`).

Anda dapat menggunakan script otomatis yang telah disediakan di folder proyek:
- **Kompilasi & Upload:**
  ```bash
  ./build_and_upload.sh
  ```
- **Monitor Serial (Log & IP Address):**
  ```bash
  ./monitor.sh
  ```
  *(Atau tentukan port custom: `./monitor.sh /dev/ttyUSB1`)*

Atau manual via `arduino-cli`:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
arduino-cli monitor -p /dev/ttyUSB0 -b 115200
```

---

## 4. Cara Pembaruan Firmware (Update Firmware)
Setelah firmware terpasang pertama kali melalui USB dan ESP32 berhasil terhubung ke WiFi, Anda tidak perlu lagi mencolokkan kabel USB ke perangkat untuk update berikutnya. Terdapat 2 cara update:

### A. Update via Web Browser (Web OTA)
1. Perhatikan alamat **IP Address** yang tertera pada layar OLED (atau melalui Serial Monitor), contoh: `192.168.1.50`.
2. Buka browser di komputer atau smartphone yang terhubung ke jaringan WiFi yang sama.
3. Ketik URL di browser:
   ```text
   http://192.168.1.50
   ```
4. Anda akan melihat halaman web **"ESP32 Web Firmware Update"**.
5. Klik **Choose File** / **Browse**, lalu pilih file biner hasil kompilasi (`.bin`).
   *(File `.bin` dapat ditemukan di folder cache arduino, misal: `~/.cache/arduino/sketches/.../esp32_oled_ota.ino.bin` atau export compiled binary dari Arduino IDE)*.
6. Klik tombol **Update Firmware**.
7. Layar OLED akan menampilkan progres persentase update (`Web OTA Updating... XX%`).
8. Perangkat akan otomatis *restart* setelah sukses.

### B. Update via ArduinoOTA (Network Port / CLI)
1. **Via Arduino IDE:**
   - Buka Arduino IDE.
   - Pilih menu **Tools > Port**, lalu pilih port jaringan: `esp32-lolin32-ota at <IP_ADDRESS>`.
   - Klik tombol **Upload**.
2. **Via `arduino-cli`:**
   ```bash
   arduino-cli upload -p esp32-lolin32-ota.local --fqbn esp32:esp32:esp32 .
   ```
   *(Atau gunakan IP Address langsung sebagai port `-p`).*

---

## 5. Indikator & Informasi pada Layar OLED
Layar OLED berukuran 128x64 piksel menampilkan informasi real-time secara bergantian:
- **Saat Booting:** Menampilkan status inisialisasi I2C dan koneksi WiFi (`Connecting to WiFi...`, IP Address).
- **Saat Berjalan (`loop`):** Menampilkan Hostname (`ESP32 Lolin32 WebOTA`), alamat IP lokal, kekuatan sinyal WiFi (`RSSI`), dan waktu aktif (*Uptime*).
- **Saat Update OTA / Web OTA:** Menampilkan baris progres visual (Progress Bar) beserta persentase (`%`) proses penulisan flash.
