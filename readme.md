# UGM Library Management System - OOP Mini Project 
Baihaqi Mustafa Surya Atmaja
20/456839/TK/50663 

---
## How to Build & Run
Aplikasi ini menggunakan `Makefile` untuk mempermudah proses kompilasi, dan dikembangkan menggunakan Linux/WSL sehingga memerlukan *compiler* `g++` yang mendukung standar `C++17`.

Buka terminal lalu arahkan ke direktori proyek ini. Kemudian jalankan perintah berikut: 
```bash 
 make
 ``` 
Perintah ini akan melakukan kompulasi terhadap semua file yang ada dalam direktori `src/` dan menghasilkan file eksekusi binary dengan nama `admin`.
Setelah selesai dan tidak ditemukan error, jalankan aplikasi dengan membuka file admin di terminal.
```bash
./admin
```
Setelah dijalankan, dua interface akan muncul.
1. Admin CLI 
Gunakan username `admin` dan password `admin123` untuk masuk ke admin console.
2. User Web Interface
Akses Web sebagai user melalui `http://localhost:8080`

## UML Diagram

<img width="1934" height="518" alt="bLPHRzCm47wlrFzWxG4osWuatYfKhOCzL6BJDFQEdFYMcYPsiPr2GUEtx-mqYUEu7UjR_7rt_huxdzrBRQWoLLcCHth3nPOgMfAie5elZL4yhGoiCYc-yJz0FlYSKWgPhPKiWHXLWG_fDMNo5nSvUQI5HcWysj9iGtCWPzV2SBCdxycDP52SaR_Z4S6Vsv5SIRbfLknlHZZxFYTS67yD4nI" src="https://github.com/user-attachments/assets/afb1d9fe-c798-4f8d-b956-61090465ae93" />


## Limitation
 - Mekanisme penggunaan pada halaman web tidak menggunakan sistem otentikasi, sehingga siapapun yang mengetahui ID seseorang yang terdaftar dapat melakukan peminjaman atas nama ID tersebut.
 - Pengembangan Web menggunakan penggabungan HTML secara manual dalam `main.cpp` sehingga file c++ tersebut menjadi sangat panjang (sulit dimanage).
