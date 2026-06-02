#include "Database.h"
#include <iostream>

Database::Database(const std::string& dbFile) {
    int rc = sqlite3_open(dbFile.c_str(), &connection_);
    if (rc != SQLITE_OK) {
        std::cerr << "Gagal membuka database: " << sqlite3_errmsg(connection_) << std::endl;
    } else {
        std::cout << "[OK] Koneksi database berhasil dibuka." << std::endl;
    }
}

Database::~Database() {
    // Menutup koneksi secara otomatis
    if (connection_) {
        sqlite3_close(connection_);
        std::cout << "[OK] Koneksi database ditutup." << std::endl;
    }
}

bool Database::exec(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(connection_, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

sqlite3* Database::getConnection() const {
    return connection_;
}