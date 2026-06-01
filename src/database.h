#pragma once
#include <sqlite3.h> // C API untuk SQLite
#include <string>

class Database {
private:
    sqlite3* connection_; // Handle database

public:
    // Constructor akan membuka koneksi (RAII)
    Database(const std::string& dbFile = "data.db");
    
    // Destructor akan menutup koneksi (RAII)
    ~Database();

    // Mengeksekusi perintah SQL (seperti CREATE TABLE atau INSERT)
    bool exec(const std::string& sql);

    // Mengambil raw connection untuk query yang lebih kompleks (SELECT)
    sqlite3* getConnection() const;
};