#include <iostream>
#include <string>
#include <termios.h> 
#include <unistd.h>
#include <iomanip>
#include <limits>
#include "Database.h"
#include "Repository.h"

// Fungsi helper untuk menyamarkan input password di terminal Linux/WSL
std::string getHiddenPassword() {
    std::string password;
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt); // Simpan pengaturan terminal saat ini
    
    termios newt = oldt;
    newt.c_lflag &= ~ECHO; // Matikan fitur echo (menampilkan ketikan)
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cin >> password;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Kembalikan pengaturan terminal seperti semula
    std::cout << std::endl; // Tambahkan enter karena tombol enter saat echo mati tidak terlihat
    return password;
}

// Fungsi untuk mencocokkan username dan password ke tabel users di database
bool verifyLogin(Database& db, const std::string& username, const std::string& password) {
    std::string sql = "SELECT count(*) FROM users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;
    bool isValid = false;

    // Menyiapkan statement SQL agar aman dari injeksi
    if (sqlite3_prepare_v2(db.getConnection(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count > 0) isValid = true;
        }
        sqlite3_finalize(stmt);
    }
    return isValid;
}

void manageBooks(BookRepository& bookRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                    MANAGE BOOKS                      \n";
        std::cout << "======================================================\n";
        
        // 1. Tampilkan Daftar Buku (Format Tabel)
        auto books = bookRepo.listAll();
        std::cout << std::left << std::setw(5) << "ID" 
                  << std::setw(30) << "Title" 
                  << std::setw(20) << "Author" 
                  << "Status\n";
        std::cout << "------------------------------------------------------\n";
        for (const auto& b : books) {
            std::cout << std::left << std::setw(5) << b.getId() 
                      << std::setw(30) << b.getTitle() 
                      << std::setw(20) << b.getAuthor() 
                      << (b.isAvailable() ? "available" : "on loan") << "\n";
        }
        std::cout << "------------------------------------------------------\n";

        // 2. Opsi Aksi CRUD
        std::cout << "Action [a]dd, [e]dit, [d]elete, [b]ack: ";
        std::cin >> action;

        // Membersihkan buffer cin untuk menghindari error getline setelahnya
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (tolower(action)) {
            case 'a': {
                int id;
                std::string title, author;
                
                std::cout << ">> Tambah Buku Baru\nMasukkan ID (Angka): ";
                // Validasi agar input ID tidak error jika diisi huruf
                while (!(std::cin >> id)) {
                    std::cout << "Input tidak valid! Masukkan angka untuk ID: ";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                try {
                    // Cek apakah ID sudah ada
                    bookRepo.findById(id);
                    std::cout << "[!] Gagal: Buku dengan ID " << id << " sudah terdaftar.\n";
                } catch (...) {
                    // Jika tidak ditemukan (masuk catch), berarti ID aman dipakai
                    std::cout << "Masukkan Judul: ";
                    std::getline(std::cin, title);
                    std::cout << "Masukkan Penulis: ";
                    std::getline(std::cin, author);
                    
                    Book newBook(id, title, author);
                    bookRepo.save(newBook);
                    std::cout << "[OK] Buku berhasil ditambahkan.\n";
                }
                break;
            }
            case 'e': {
                int id;
                std::cout << ">> Edit Buku\nMasukkan ID Buku yang akan diedit: ";
                if (std::cin >> id) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    try {
                        Book b = bookRepo.findById(id);
                        std::cout << "Judul saat ini: " << b.getTitle() << "\n";
                        std::cout << "Judul Baru (kosongkan jika tidak ingin diubah): ";
                        std::string newTitle;
                        std::getline(std::cin, newTitle);
                        
                        std::cout << "Penulis saat ini: " << b.getAuthor() << "\n";
                        std::cout << "Penulis Baru (kosongkan jika tidak ingin diubah): ";
                        std::string newAuthor;
                        std::getline(std::cin, newAuthor);

                        // Gunakan data baru jika diisi, jika kosong gunakan data lama
                        std::string finalTitle = newTitle.empty() ? b.getTitle() : newTitle;
                        std::string finalAuthor = newAuthor.empty() ? b.getAuthor() : newAuthor;
                        
                        Book updatedBook(id, finalTitle, finalAuthor);
                        updatedBook.setAvailable(b.isAvailable()); // Pertahankan status ketersediaan
                        bookRepo.save(updatedBook);
                        std::cout << "[OK] Buku berhasil diperbarui.\n";
                    } catch (const std::exception& e) {
                        std::cout << "[!] " << e.what() << "\n";
                    }
                }
                break;
            }
            case 'd': {
                int id;
                std::cout << ">> Hapus Buku\nMasukkan ID Buku yang akan dihapus: ";
                if (std::cin >> id) {
                    try {
                        Book b = bookRepo.findById(id);
                        // Validasi tambahan: Buku yang sedang dipinjam tidak boleh dihapus 
                        if (!b.isAvailable()) {
                            std::cout << "[!] Gagal: Buku sedang dipinjam dan tidak dapat dihapus.\n";
                        } else {
                            bookRepo.remove(id);
                            std::cout << "[OK] Buku dengan ID " << id << " berhasil dihapus.\n";
                        }
                    } catch (const std::exception& e) {
                         std::cout << "[!] " << e.what() << "\n";
                    }
                }
                break;
            }
            case 'b':
                inMenu = false;
                break;
            default:
                std::cout << "[!] Pilihan tidak valid.\n";
        }
    }
}

// === FUNGSI SUB-MENU MANAGE MEMBERS ===
void manageMembers(MemberRepository& memberRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                    MANAGE MEMBERS                    \n";
        std::cout << "======================================================\n";
        
        // 1. Tampilkan Daftar Anggota
        auto members = memberRepo.listAll();
        std::cout << std::left << std::setw(10) << "ID" 
                  << std::setw(25) << "Name" 
                  << "Email\n";
        std::cout << "------------------------------------------------------\n";
        for (const auto& m : members) {
            std::cout << std::left << std::setw(10) << m.getId() 
                      << std::setw(25) << m.getName() 
                      << m.getEmail() << "\n";
        }
        std::cout << "------------------------------------------------------\n";

        // 2. Opsi Aksi CRUD
        std::cout << "Action [a]dd, [e]dit, [d]elete, [b]ack: ";
        std::cin >> action;

        // Membersihkan buffer cin
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (tolower(action)) {
            case 'a': {
                std::string id, name, email;
                
                std::cout << ">> Tambah Anggota Baru\nMasukkan ID (misal M003): ";
                std::getline(std::cin, id);

                try {
                    // Cek apakah ID sudah ada
                    memberRepo.findById(id);
                    std::cout << "[!] Gagal: Anggota dengan ID " << id << " sudah terdaftar.\n";
                } catch (...) {
                    std::cout << "Masukkan Nama: ";
                    std::getline(std::cin, name);
                    std::cout << "Masukkan Email: ";
                    std::getline(std::cin, email);
                    
                    Member newMember(id, name, email);
                    memberRepo.save(newMember);
                    std::cout << "[OK] Anggota berhasil ditambahkan.\n";
                }
                break;
            }
            case 'e': {
                std::string id;
                std::cout << ">> Edit Anggota\nMasukkan ID Anggota yang akan diedit: ";
                std::getline(std::cin, id);
                try {
                    Member m = memberRepo.findById(id);
                    std::cout << "Nama saat ini: " << m.getName() << "\n";
                    std::cout << "Nama Baru (kosongkan jika tidak ingin diubah): ";
                    std::string newName;
                    std::getline(std::cin, newName);
                    
                    std::cout << "Email saat ini: " << m.getEmail() << "\n";
                    std::cout << "Email Baru (kosongkan jika tidak ingin diubah): ";
                    std::string newEmail;
                    std::getline(std::cin, newEmail);

                    // Gunakan data baru jika diisi, jika kosong gunakan data lama
                    std::string finalName = newName.empty() ? m.getName() : newName;
                    std::string finalEmail = newEmail.empty() ? m.getEmail() : newEmail;
                    
                    Member updatedMember(id, finalName, finalEmail);
                    memberRepo.save(updatedMember);
                    std::cout << "[OK] Data anggota berhasil diperbarui.\n";
                } catch (const std::exception& e) {
                    std::cout << "[!] " << e.what() << "\n";
                }
                break;
            }
            case 'd': {
                std::string id;
                std::cout << ">> Hapus Anggota\nMasukkan ID Anggota yang akan dihapus: ";
                std::getline(std::cin, id);
                try {
                    Member m = memberRepo.findById(id);
                    memberRepo.remove(id);
                    std::cout << "[OK] Anggota dengan ID " << id << " berhasil dihapus.\n";
                } catch (const std::exception& e) {
                     std::cout << "[!] " << e.what() << "\n";
                }
                break;
            }
            case 'b':
                inMenu = false;
                break;
            default:
                std::cout << "[!] Pilihan tidak valid.\n";
        }
    }
}

// === FUNGSI SUB-MENU ISSUE / RETURN LOAN ===
void manageLoans(LoanRepository& loanRepo, BookRepository& bookRepo, MemberRepository& memberRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                 ISSUE / RETURN LOAN                  \n";
        std::cout << "======================================================\n";
        
        auto activeLoans = loanRepo.listActiveLoans();
        std::cout << std::left << std::setw(5) << "ID" 
                  << std::setw(25) << "Book Title" 
                  << std::setw(20) << "Member Name" 
                  << "Due Date\n";
        std::cout << "------------------------------------------------------\n";
        if (activeLoans.empty()) {
            std::cout << "Tidak ada peminjaman aktif saat ini.\n";
        } else {
            for (const auto& l : activeLoans) {
                std::cout << std::left << std::setw(5) << l.loanId 
                          << std::setw(25) << l.bookTitle.substr(0, 23) 
                          << std::setw(20) << l.memberName.substr(0, 18) 
                          << l.dueDate << "\n";
            }
        }
        std::cout << "------------------------------------------------------\n";

        std::cout << "Action [i]ssue, [r]eturn, [b]ack: ";
        std::cin >> action;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (tolower(action)) {
            case 'i': {
                int bookId;
                std::string memberId;
                
                std::cout << ">> Peminjaman Buku\nMasukkan ID Buku: ";
                if (!(std::cin >> bookId)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "[!] ID Buku tidak valid.\n";
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                std::cout << "Masukkan ID Anggota: ";
                std::getline(std::cin, memberId);

                try {
                    // Validasi apakah buku dan member benar-benar ada
                    Book b = bookRepo.findById(bookId);
                    Member m = memberRepo.findById(memberId);
                    
                    if (!b.isAvailable()) {
                        std::cout << "[!] Gagal: Buku sedang dipinjam oleh orang lain.\n";
                    } else {
                        loanRepo.issueLoan(bookId, memberId);
                        std::cout << "[OK] Buku berhasil dipinjamkan hingga 7 hari ke depan.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "[!] Error: " << e.what() << "\n";
                }
                break;
            }
            case 'r': {
                int bookId;
                std::cout << ">> Pengembalian Buku\nMasukkan ID Buku yang dikembalikan: ";
                if (std::cin >> bookId) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    try {
                        Book b = bookRepo.findById(bookId);
                        if (b.isAvailable()) {
                            std::cout << "[!] Gagal: Buku tersebut tidak sedang dipinjam (tersedia).\n";
                        } else {
                            loanRepo.returnLoan(bookId);
                            std::cout << "[OK] Buku berhasil dikembalikan ke perpustakaan.\n";
                        }
                    } catch (const std::exception& e) {
                        std::cout << "[!] Error: " << e.what() << "\n";
                    }
                } else {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "[!] Input tidak valid.\n";
                }
                break;
            }
            case 'b':
                inMenu = false;
                break;
            default:
                std::cout << "[!] Pilihan tidak valid.\n";
        }
    }
}

int main() {
    Database db("data.db");

    // === 1. SETUP DATABASE ===
    std::string createTableSQL = 
        "CREATE TABLE IF NOT EXISTS books (" 
        "id INTEGER PRIMARY KEY, " 
        "title TEXT NOT NULL, " 
        "author TEXT NOT NULL, " 
        "available INTEGER DEFAULT 1);"
        
        "CREATE TABLE IF NOT EXISTS users (" 
        "username TEXT PRIMARY KEY, "
        "password_hash TEXT NOT NULL, "
        "role TEXT NOT NULL);"

        "CREATE TABLE IF NOT EXISTS members ("
        "id TEXT PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "email TEXT NOT NULL);"

        "CREATE TABLE IF NOT EXISTS loans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "book_id INTEGER, "
        "member_id TEXT, "
        "due_date TEXT NOT NULL, "
        "FOREIGN KEY(book_id) REFERENCES books(id), "
        "FOREIGN KEY(member_id) REFERENCES members(id));";
    db.exec(createTableSQL);

    // std::string seedDataSQL = 
    //     "INSERT OR IGNORE INTO books (id, title, author, available) VALUES "
    //     "(1, 'Laskar Pelangi', 'Nidji', 1), "
    //     "(2, 'Kicau Mania', 'Ndarboy Genk', 1), "
    //     "(3, '67', 'brainrot', 0), "
    //     "(4, 'Skripsi susah? Ternak lele aja', 'Mahasiswa desperate', 1), "
    //     "(5, 'Trik Sukses Bisnis Angkringan', 'Yanto GOR Pancasila', 1);"

    //     "INSERT OR IGNORE INTO users (username, password_hash, role) VALUES ('admin', 'admin123', 'admin');"

    //     "INSERT OR IGNORE INTO members (id, name, email) VALUES "
    //     "('M001', 'Budi Santoso', 'budi@example.com'), "
    //     "('M002', 'Siti Aminah', 'siti@example.com');";
    // db.exec(seedDataSQL);

    // === SISTEM LOGIN CLI ===
    std::cout << "== Library Admin Console ==" << std::endl; 

    std::string username, password;
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;
    bool loggedIn = false;

    // Loop login maksimal 3 kali 
    while (attempts < MAX_ATTEMPTS) {
        std::cout << "Username: ";
        std::cin >> username;
        
        std::cout << "Password: ";
        password = getHiddenPassword(); // Memanggil fungsi penyamar password

        if (verifyLogin(db, username, password)) {
            loggedIn = true;
            break; // Keluar dari loop jika login berhasil
        } else {
            attempts++;
            std::cout << "Login gagal! Sisa percobaan: " << (MAX_ATTEMPTS - attempts) << "\n" << std::endl;
        }
    }

    // Jika gagal 3 kali, blokir akses dan hentikan program 
    if (!loggedIn) {
        std::cout << "Akses diblokir. Terlalu banyak percobaan gagal." << std::endl;
        return 1; 
    }

    std::cout << "[OK] Login OK. Welcome, " << username << ".\n" << std::endl; // 

    // === 3. MENU UTAMA ===
    BookRepository bookRepo(db);
    MemberRepository memberRepo(db);
    LoanRepository loanRepo(db);
    char choice;
    bool running = true;

    while (running) {
        std::cout << "\n=========================================\n";
        std::cout << "        UGM Library Admin Console        \n";
        std::cout << "=========================================\n";
        std::cout << "[1] Manage books\n";
        std::cout << "[2] Manage members\n";
        std::cout << "[3] Issue / return loan\n";
        std::cout << "[Q] Quit\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        switch (choice) {
            case '1':
                manageBooks(bookRepo);
                break;
            case '2':
                manageMembers(memberRepo);
                break;
            case '3':
                manageLoans(loanRepo, bookRepo, memberRepo);
                break;
            case 'Q':
            case 'q':
                std::cout << "\nKeluar dari sistem. Terima kasih, " << username << "!\n";
                running = false; // Menghentikan perulangan
                break;
            default:
                std::cout << "\n[!] Pilihan tidak valid. Silakan masukkan 1, 2, 3, atau Q.\n";
        }
    }
    return 0;
}