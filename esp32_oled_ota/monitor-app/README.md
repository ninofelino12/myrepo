# ESP32 RTDB Monitor Application

Aplikasi Express.js modern untuk memantau data log dan status device dari Firebase Realtime Database (RTDB) berdasarkan konfigurasi dari file `.env`.

## Fitur
- **Dashboard Modern & Responsif**: Dibangun dengan Tailwind CSS dan tema gelap (dark mode).
- **Auto-Refresh**: Mendukung polling otomatis setiap 5 detik untuk pembaruan data secara real-time.
- **Monitoring Data**:
  - Live Boot Events Log (`/bootEvents`)
  - Status & Informasi Device Terdaftar (`/devices`)
- **Konfigurasi Fleksibel**: Membaca URL RTDB dan API Key langsung dari file `.env` / `rtdb.env`.

---

## Cara Instalasi & Menjalankan (Lokal)

1. Masuk ke direktori `monitor-app`:
   ```bash
   cd monitor-app
   ```

2. Install dependensi Node.js:
   ```bash
   npm install
   ```

3. Jalankan aplikasi server:
   ```bash
   npm start
   ```

4. Buka browser di:
   ```text
   http://localhost:3000
   ```

---

## Deployment ke Vercel

Aplikasi ini sudah dikonfigurasi untuk siap dideploy ke **Vercel** menggunakan `vercel.json`.

1. Pastikan Vercel CLI sudah terinstal (atau gunakan Vercel Dashboard):
   ```bash
   npm install -g vercel
   ```

2. Login ke Vercel:
   ```bash
   vercel login
   ```

3. Masuk ke folder `monitor-app` dan deploy:
   ```bash
   cd monitor-app
   vercel
   ```

4. **Environment Variables di Vercel**:
   Pastikan Anda menambahkan Environment Variables berikut di Vercel Dashboard (Project Settings -> Environment Variables) atau saat prompt CLI:
   - `RTDB_URL` = `https://bizflow-dashboard-vlo72-default-rtdb.asia-southeast1.firebasedatabase.app`
   - `API_KEY` = `AIzaSyAvWCSjfvz4R5kduVDtTiPiPrY-nUJki6o`
