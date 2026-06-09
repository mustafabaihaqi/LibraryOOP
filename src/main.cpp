#include <iostream>
#include <string>
#include <termios.h> 
#include <unistd.h>
#include <iomanip>
#include <limits>
#include <thread>
#include "httplib.h"
#include "database.h"
#include "repository.h"

// Fungsi untuk menyamarkan password di terminal
std::string getHiddenPassword() {
    std::string password;
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt); 
    
    termios newt = oldt;
    newt.c_lflag &= ~ECHO; 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cin >> password;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl; 
    return password;
}

// Fungsi untuk mencocokkan username dan password ke tabel users di database
bool verifyLogin(Database& db, const std::string& username, const std::string& password) {
    std::string sql = "SELECT count(*) FROM users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;
    bool isValid = false;

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

// Halaman Manage Books di CLI
void manageBooks(BookRepository& bookRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                    MANAGE BOOKS                      \n";
        std::cout << "======================================================\n";
        
        // Menampilkan Daftar Buku
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

        std::cout << "Action [a]dd, [e]dit, [d]elete, [b]ack: ";
        std::cin >> action;

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
            // Fitur Edit Buku
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
                        updatedBook.setAvailable(b.isAvailable()); 
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
                        // Buku yang sedang dipinjam tidak boleh dihapus 
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

// Halaman Manage Members di CLI
void manageMembers(MemberRepository& memberRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                    MANAGE MEMBERS                    \n";
        std::cout << "======================================================\n";
        
        // Tampilkan Daftar Anggota
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

        std::cout << "Action [a]dd, [e]dit, [d]elete, [b]ack: ";
        std::cin >> action;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (tolower(action)) {
            case 'a': {
                std::string id, name, email;
                
                std::cout << ">> Tambah Anggota Baru\nMasukkan ID: ";
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

// Halaman Issue / Return Loan
void manageLoans(LoanRepository& loanRepo, BookRepository& bookRepo, MemberRepository& memberRepo) {
    char action;
    bool inMenu = true;

    while (inMenu) {
        std::cout << "\n======================================================\n";
        std::cout << "                 ISSUE / RETURN LOAN                  \n";
        std::cout << "======================================================\n";
        
        // Tampilkan Buku yang Dipinjam
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

// CSS untuk web
std::string getCommonCSS() {
    return "<style>"
           "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f6f9; }"
           ".header { background-color: #00416b; color: white; padding: 20px; display: flex; justify-content: space-between; align-items: center; }"
           ".header h1 { margin: 0; font-size: 24px; }"
           ".container { max-width: 800px; margin: 30px auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }"
           ".search-bar { margin-bottom: 20px; display: flex; gap: 10px; }"
           ".search-bar input[type='text'] { flex: 1; padding: 8px; border: 1px solid #ccc; border-radius: 4px; }"
           ".search-bar button { padding: 8px 15px; background-color: #00416b; color: white; border: none; border-radius: 4px; cursor: pointer; }"
           ".book-card { display: flex; justify-content: space-between; align-items: center; padding: 15px; border: 1px solid #e0e0e0; margin-bottom: 10px; border-radius: 4px; }"
           ".book-title { font-weight: bold; font-size: 16px; }"
           ".book-author { color: #666; font-style: italic; margin-left: 10px; }"
           ".status-badge { padding: 4px 8px; border-radius: 4px; font-size: 12px; font-weight: bold; color: white; }"
           ".status-available { background-color: #28a745; }"
           ".status-loan { background-color: #6c757d; }"
           ".btn-return { background-color: #dc3545; color: white; border: none; padding: 6px 12px; border-radius: 4px; cursor: pointer; }"
           ".nav-link { color: white; text-decoration: none; font-weight: bold; margin-left: 15px; }"
           "</style>";
}

// === FUNGSI WEB SERVER UTAMA ===
void startWebServer(BookRepository& bookRepo, MemberRepository& memberRepo, LoanRepository& loanRepo) {
    httplib::Server svr;

    // Landing Page (Daftar Semua Buku)
    svr.Get("/", [&bookRepo](const httplib::Request&, httplib::Response& res) {
        std::string html = "<html><head><title>UGM Library</title>" + getCommonCSS() + "</head><body>";
        html += "<div class='header'><h1>Perpustakaan UGM</h1>"
                "<div><a class='nav-link' href='/'>Katalog</a>"
                "<form action='/me' method='GET' style='display:inline; margin-left:15px;'>"
                "<input type='text' name='id' placeholder='Masukkan NIM' required style='padding:4px; border-radius:4px; border:none;'>"
                "<button type='submit' style='padding:4px 8px; margin-left:5px; cursor:pointer;'>Go</button>"
                "</form></div></div>";
        
        html += "<div class='container'>";
        html += "<form class='search-bar' action='/search' method='GET'>"
                "<input type='text' name='q' placeholder='Cari judul atau penulis...' required>"
                "<button type='submit'>Cari</button>"
                "</form>";

        auto books = bookRepo.listAll();
        for (const auto& b : books) {
            html += "<div class='book-card'><div>";
            html += "<span class='book-title'>" + b.getTitle() + "</span>";
            html += "<span class='book-author'>" + b.getAuthor() + "</span></div>";
            
            if (b.isAvailable()) {
                // Form peminjaman langsung di tempat dengan memasukkan ID Anggota
                html += "<form action='/borrow' method='POST' style='margin:0; display:flex; gap:5px;'>";
                html += "<input type='hidden' name='book_id' value='" + std::to_string(b.getId()) + "'>";
                html += "<input type='text' name='member_id' placeholder='Masukkan NIM' required style='padding:5px; border:1px solid #ccc; border-radius:4px; font-size:12px;'>";
                html += "<button type='submit' style='background-color:#28a745; color:white; border:none; padding:5px 10px; border-radius:4px; cursor:pointer; font-weight:bold;'>Pinjam</button>";
                html += "</form>";
            } else {
                html += "<span class='status-badge status-loan'>Dipinjam</span>";
            }
            html += "</div>";
        }
        html += "</div></body></html>";
        res.set_content(html, "text/html");
    });

    // Fitur Pencarian Kata Kunci Sederhana
    svr.Get("/search", [&bookRepo](const httplib::Request& req, httplib::Response& res) {
        std::string query = req.get_param_value("q");
        // Mengubah query ke lowercase untuk pencarian case-insensitive sederhana
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

        std::string html = "<html><head><title>Hasil Pencarian</title>" + getCommonCSS() + "</head><body>";
        html += "<div class='header'><h1>Hasil Pencarian untuk: \"" + req.get_param_value("q") + "\"</h1><a class='nav-link' href='/'>&larr; Kembali</a></div>";
        html += "<div class='container'>";

        auto books = bookRepo.listAll();
        int foundCount = 0;
        for (const auto& b : books) {
            std::string titleLower = b.getTitle();
            std::string authorLower = b.getAuthor();
            std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            std::transform(authorLower.begin(), authorLower.end(), authorLower.begin(), ::tolower);

            if (titleLower.find(query) != std::string::npos || authorLower.find(query) != std::string::npos) {
                foundCount++;
                html += "<div class='book-card'><div>";
                html += "<span class='book-title'>" + b.getTitle() + "</span>";
                html += "<span class='book-author'>" + b.getAuthor() + "</span></div>";
                html += (b.isAvailable() ? "<span class='status-badge status-available'>Tersedia</span>" : "<span class='status-badge status-loan'>On Loan</span>");
                html += "</div>";
            }
        }

        if (foundCount == 0) {
            html += "<p style='text-align:center; color:#666;'>Buku tidak ditemukan. Coba kata kunci lain.</p>";
        }
        html += "</div></body></html>";
        res.set_content(html, "text/html");
    });

    // Memproses Pinjam Buku dari Web
    svr.Post("/borrow", [&bookRepo, &memberRepo, &loanRepo](const httplib::Request& req, httplib::Response& res) {
        std::string bookIdStr = req.get_param_value("book_id");
        std::string memberId = req.get_param_value("member_id");

        try {
            int bookId = std::stoi(bookIdStr);
            // Validasi apakah NIM/ID Anggota terdaftar di DB
            memberRepo.findById(memberId); 
            Book b = bookRepo.findById(bookId);

            if (b.isAvailable()) {
                loanRepo.issueLoan(bookId, memberId);
                // Jika sukses, langsung arahkan ke halaman pinjaman milik member tersebut
                res.set_redirect("/me?id=" + memberId);
            } else {
                res.set_content("<h1>[!] Gagal: Buku sudah dipinjam orang lain.</h1><a href='/'>Kembali</a>", "text/html");
            }
        } catch (...) {
            res.set_content("<h1>[!] Error: NIM/ID Anggota tidak terdaftar atau data salah!</h1><a href='/'>Kembali Ke Katalog</a>", "text/html");
        }
    });

    // Halaman Web Daftar Pinjaman Aktif Anggota
    svr.Get("/me", [&memberRepo, &loanRepo](const httplib::Request& req, httplib::Response& res) {
        std::string memberId = req.get_param_value("id");
        
        std::string html = "<html><head><title>Pinjamanku</title>" + getCommonCSS() + "</head><body>";
        try {
            auto member = memberRepo.findById(memberId);
            html += "<div class='header'><h1>Pinjamanku</h1><span class='nav-link'>Halo, " + member.getName() + " (" + member.getId() + ")</span></div>";
            html += "<div class='container'>";
            html += "<h3>Buku yang Dipinjam:</h3>";

            auto allLoans = loanRepo.listActiveLoans();
            int loanCount = 0;
            
            for (const auto& l : allLoans) {
                // menampilkan buku yang dipinjam oleh anggota dengan ID
                if (l.memberId == memberId) {
                    loanCount++;
                    html += "<div class='book-card'><div>";
                    html += "<span class='book-title'>" + l.bookTitle + "</span><br>";
                    html += "<small style='color:#666;'>Kembalikan Sebelum: " + l.dueDate + "</small></div>";
                    
                    // Tombol Return untuk mengembalikan langsung dari web
                    html += "<form action='/return' method='POST' style='margin:0;'>";
                    html += "<input type='hidden' name='book_id' value='" + std::to_string(l.bookId) + "'>";
                    html += "<input type='hidden' name='member_id' value='" + memberId + "'>";
                    html += "<button type='submit' class='btn-return'>Kembalikan</button>";
                    html += "</form>";
                    html += "</div>";
                }
            }

            if (loanCount == 0) {
                html += "<p style='color:#666;'>Kamu tidak sedang meminjam buku apapun saat ini.</p>";
            }
            
            html += "<br><a href='/' style='display:inline-block; text-decoration:none; color:#0066b2; font-weight:bold;'>&larr; Kembali ke Katalog Utama</a>";
        } catch (...) {
            html += "<div class='header'><h1>Error</h1><a class='nav-link' href='/'>Kembali</a></div>";
            html += "<div class='container'><p style='color:red; text-align:center;'>[!] Profil gagal dimuat. ID Anggota / NIM tidak valid atau tidak ditemukan.</p></div>";
        }

        html += "</div></body></html>";
        res.set_content(html, "text/html");
    });

    // Pengembalian Buku dari Web
    svr.Post("/return", [&loanRepo](const httplib::Request& req, httplib::Response& res) {
        std::string bookIdStr = req.get_param_value("book_id");
        std::string memberId = req.get_param_value("member_id");

        try {
            int bookId = std::stoi(bookIdStr);
            loanRepo.returnLoan(bookId);
            res.set_redirect("/me?id=" + memberId);
        } catch (...) {
            res.set_redirect("/");
        }
    });

    svr.listen("localhost", 8080);
}

int main() {
    Database db("data.db");

    sqlite3_busy_timeout(db.getConnection(), 1000);
    //=== 1. SETUP DATABASE ===
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

    std::string seedDataSQL = 
        "INSERT OR IGNORE INTO books (id, title, author, available) VALUES "
        "(1, 'Laskar Pelangi', 'Nidji', 1), "
        "(2, 'Kicau Mania', 'Ndarboy Genk', 1), "
        "(3, '67', 'brainrot', 1), "
        "(4, 'Skripsi susah? Ternak lele aja', 'Mahasiswa desperate', 1), "
        "(5, 'Trik Sukses Bisnis Angkringan', 'Yanto GOR Pancasila', 1);"

        "INSERT OR IGNORE INTO users (username, password_hash, role) VALUES ('admin', 'admin123', 'admin');"

        "INSERT OR IGNORE INTO members (id, name, email) VALUES "
        "('M001', 'Budi Santoso', 'budi@example.com'), "
        "('M002', 'Siti Aminah', 'siti@example.com');";
    db.exec(seedDataSQL);

    // Menyalakan Web Server di background
    BookRepository bookRepo(db);
    MemberRepository memberRepo(db);
    LoanRepository loanRepo(db);
    std::cout << ">> Memulai Web Server (http://localhost:8080)...\n";
    std::thread webThread(startWebServer, std::ref(bookRepo), std::ref(memberRepo), std::ref(loanRepo));
    webThread.detach();

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
                running = false; 
                break;
            default:
                std::cout << "\n[!] Pilihan tidak valid. Silakan masukkan 1, 2, 3, atau Q.\n";
        }
    }
    return 0;
}