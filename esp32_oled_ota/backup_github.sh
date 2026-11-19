#!/bin/bash

# --- Konfigurasi ---
BACKUP_DIR="github_backup_$(date +%Y%m%d_%H%M%S)"
TARGET_FILE="github_environment_backup.tar.gz"

echo "🚀 Memulai backup lingkungan GitHub..."

# 1. Buat folder sementara
mkdir -p "$BACKUP_DIR"

# 2. Backup Git Config
if [ -f ~/.gitconfig ]; then
    echo "📦 Menyalin .gitconfig..."
    cp ~/.gitconfig "$BACKUP_DIR/gitconfig"
fi

# 3. Backup SSH Keys
if [ -d ~/.ssh ]; then
    echo "📦 Menyalin folder .ssh..."
    cp -r ~/.ssh "$BACKUP_DIR/ssh"
fi

# 4. Backup GitHub CLI (gh) config
if [ -d ~/.config/gh ]; then
    echo "📦 Menyalin konfigurasi GitHub CLI..."
    mkdir -p "$BACKUP_DIR/gh_config"
    cp -r ~/.config/gh/* "$BACKUP_DIR/gh_config/"
fi

# 5. Backup file .env dari workspace saat ini (opsional tapi disarankan)
echo "🔍 Mencari file .env di direktori saat ini..."
mkdir -p "$BACKUP_DIR/env_files"
find . -maxdepth 3 -name ".env*" -not -path "*/node_modules/*" -exec cp --parents {} "$BACKUP_DIR/env_files/" \;

# 6. Kompres semua menjadi satu file
echo "🗜️ Mengompresi file backup..."
tar -czvf "$TARGET_FILE" "$BACKUP_DIR"

# 7. Bersihkan folder sementara
rm -rf "$BACKUP_DIR"

echo "------------------------------------------------"
echo "✅ Backup selesai!"
echo "📄 File: $TARGET_FILE"
echo "⚠️  PERINGATAN: File ini mengandung KUNCI PRIVAT dan TOKEN AKSES."
echo "🔐 Simpan di tempat yang aman dan jangan dibagikan!"
echo "------------------------------------------------"
