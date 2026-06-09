#include "repository.h"
#include <iostream>
#include <stdexcept>

// Find by ID Buku
Book BookRepository::findById(int id) {
    std::string sql = "SELECT id, title, author, available FROM books WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Gagal menyiapkan statement findById");
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int bookId = sqlite3_column_int(stmt, 0);
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bool available = sqlite3_column_int(stmt, 3);

        Book book(bookId, title, author);
        book.setAvailable(available);
        
        sqlite3_finalize(stmt); 
        return book;
    }

    sqlite3_finalize(stmt);
    throw std::runtime_error("Buku dengan ID tersebut tidak ditemukan.");
}

// Mengambil semua buku dari database
std::vector<Book> BookRepository::listAll() {
    std::vector<Book> books;
    std::string sql = "SELECT id, title, author, available FROM books;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement listAll" << std::endl;
        return books;
    }

    // Looping selama masih ada data
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int bookId = sqlite3_column_int(stmt, 0);
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bool available = sqlite3_column_int(stmt, 3);

        Book book(bookId, title, author);
        book.setAvailable(available);
        books.push_back(book);
    }

    sqlite3_finalize(stmt);
    return books;
}

// Insert dan Update Buku
void BookRepository::save(const Book& obj) {
    // Insert or Replace untuk menambah maupun mengupdate
    std::string sql = "INSERT OR REPLACE INTO books (id, title, author, available) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement save" << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, obj.getId());
    sqlite3_bind_text(stmt, 2, obj.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, obj.getAuthor().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, obj.isAvailable() ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Gagal menyimpan buku ke database." << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Menghapus buku berdasarkan ID
void BookRepository::remove(int id) {
    std::string sql = "DELETE FROM books WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement remove" << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Gagal menghapus buku." << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Find By ID Member
Member MemberRepository::findById(const std::string& id) {
    std::string sql = "SELECT id, name, email FROM members WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Gagal menyiapkan statement findById Member");
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string memberId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        Member member(memberId, name, email);
        sqlite3_finalize(stmt);
        return member;
    }

    sqlite3_finalize(stmt);
    throw std::runtime_error("Anggota dengan ID tersebut tidak ditemukan.");
}

// Mengambil semua data member
std::vector<Member> MemberRepository::listAll() {
    std::vector<Member> members;
    std::string sql = "SELECT id, name, email FROM members;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement listAll Member" << std::endl;
        return members;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string memberId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        members.emplace_back(memberId, name, email);
    }

    sqlite3_finalize(stmt);
    return members;
}

// Insert atau update member
void MemberRepository::save(const Member& obj) {
    std::string sql = "INSERT OR REPLACE INTO members (id, name, email) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement save Member" << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, obj.getId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, obj.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, obj.getEmail().c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Gagal menyimpan data anggota ke database." << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Delete Member by ID
void MemberRepository::remove(const std::string& id) {
    std::string sql = "DELETE FROM members WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Gagal menyiapkan statement remove Member" << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Gagal menghapus anggota." << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Menyimpan Pinjaman
void LoanRepository::issueLoan(int bookId, const std::string& memberId) {
    std::string sqlInsert = "INSERT INTO loans (book_id, member_id, due_date) VALUES (?, ?, date('now', '+7 days'));";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getConnection(), sqlInsert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);
        sqlite3_bind_text(stmt, 2, memberId.c_str(), -1, SQLITE_TRANSIENT);
        
        // Pengecekan Error saat mengeksekusi Insert
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "\n[!] ALARM ERROR INSERT: " << sqlite3_errmsg(db_.getConnection()) << "\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cout << "\n[!] ALARM ERROR PREPARE INSERT: " << sqlite3_errmsg(db_.getConnection()) << "\n";
    }

    // Ubah status peminjaman buku
    std::string sqlUpdate = "UPDATE books SET available = 0 WHERE id = ?;";
    if (sqlite3_prepare_v2(db_.getConnection(), sqlUpdate.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Daftar Loans
std::vector<LoanRecord> LoanRepository::listActiveLoans() {
    std::vector<LoanRecord> activeLoans;
    std::string sql = "SELECT l.id, l.book_id, b.title, l.member_id, m.name, l.due_date "
                      "FROM loans l "
                      "JOIN books b ON l.book_id = b.id "
                      "JOIN members m ON l.member_id = m.id;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            LoanRecord record;
            record.loanId = sqlite3_column_int(stmt, 0);
            record.bookId = sqlite3_column_int(stmt, 1);
            record.bookTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            record.memberId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            record.memberName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            record.dueDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            activeLoans.push_back(record);
        }
    } else {
        // indikator error
        std::cout << "\n[!] ALARM ERROR SELECT LOANS: " << sqlite3_errmsg(db_.getConnection()) << "\n";
    }
    sqlite3_finalize(stmt);
    return activeLoans;
}

void LoanRepository::returnLoan(int bookId) {
    // Hapus data buku dari tabel loans
    std::string sqlDelete = "DELETE FROM loans WHERE book_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_.getConnection(), sqlDelete.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // Ubah status buku kembali tersedia 
    std::string sqlUpdate = "UPDATE books SET available = 1 WHERE id = ?;";
    if (sqlite3_prepare_v2(db_.getConnection(), sqlUpdate.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}