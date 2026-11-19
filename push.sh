#!/bin/bash

# ==============================================================================
# Script Otomatisasi Git Push ke GitHub
# Repository: https://github.com/ninofelino12/myrepo
# ==============================================================================

set -e

echo "=================================================="
echo "🚀 Memulai Proses Git Commit & Push ke GitHub"
echo "=================================================="

# 1. Tampilkan status git saat ini
echo "🔍 Memeriksa status perubahan..."
git status -s

# 2. Cek apakah ada perubahan
if [[ -z $(git status --porcelain) ]]; then
    echo "✨ Tidak ada perubahan baru untuk di-push. Repositori sudah mutakhir!"
    exit 0
fi

# 3. Minta pesan commit dari pengguna
echo ""
read -p "💬 Masukkan pesan commit (Tekan ENTER untuk pesan default): " COMMIT_MSG

if [ -z "$COMMIT_MSG" ]; then
    COMMIT_MSG="Update: $(date '+%Y-%m-%d %H:%M:%S')"
fi

echo ""
echo "📦 Menambahkan file yang berubah (git add)..."
git add .

echo "commit Menyimpan commit dengan pesan: \"$COMMIT_MSG\"..."
git commit -m "$COMMIT_MSG"

echo ""
echo "📤 Mengirim perubahan ke remote GitHub (git push origin main)..."
git push origin main

echo ""
echo "=================================================="
echo "✅ Berhasil! Perubahan telah ter-push ke GitHub."
echo "🔗 https://github.com/ninofelino12/myrepo"
echo "=================================================="
