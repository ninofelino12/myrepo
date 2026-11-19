# Panduan Restorasi Lingkungan GitHub

Dokumen ini menjelaskan cara merestorasi file backup yang dihasilkan oleh `backup_github.sh`.

## 📂 Persiapan
Pastikan Anda sudah memindahkan file `github_environment_backup.tar.gz` ke komputer baru.

## 🛠️ Langkah-langkah Restorasi

### 1. Ekstrak File Backup
```bash
tar -xzvf github_environment_backup.tar.gz
cd github_backup_YYYYMMDD_HHMMSS  # Sesuaikan dengan nama folder hasil ekstrak
```

### 2. Restore Git Config
```bash
cp gitconfig ~/.gitconfig
```

### 3. Restore SSH Keys
```bash
mkdir -p ~/.ssh
cp -r ssh/* ~/.ssh/
chmod 700 ~/.ssh
chmod 600 ~/.ssh/id_ed25519  # Sesuaikan jika nama file kunci berbeda
chmod 644 ~/.ssh/id_ed25519.pub
```

### 4. Restore GitHub CLI (gh) Config
```bash
mkdir -p ~/.config/gh
cp -r gh_config/* ~/.config/gh/
```

### 5. Restore File .env (Opsional)
File `.env` berada di dalam folder `env_files/`. Anda perlu memindahkannya secara manual ke folder proyek yang sesuai.

## 🧪 Verifikasi
Setelah restorasi, jalankan perintah ini untuk memastikan semuanya berfungsi:

1.  **Cek Git:** `git config --list`
2.  **Cek SSH:** `ssh -T git@github.com`
3.  **Cek GitHub CLI:** `gh auth status`

---
⚠️ **PENTING:** Selalu hapus folder hasil ekstrak setelah selesai untuk menjaga keamanan kunci privat Anda.
