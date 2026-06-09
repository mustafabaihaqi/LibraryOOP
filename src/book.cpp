#include "book.h"

// Constructor menggunakan member initializer list
Book::Book(int id, std::string title, std::string author)
    : id_(id), title_(title), author_(author), isAvailable_(true) {}

// Implementasi Getters
int Book::getId() const { return id_; }
const std::string& Book::getTitle() const { return title_; }
const std::string& Book::getAuthor() const { return author_; }
bool Book::isAvailable() const { return isAvailable_; }

// Implementasi Setters
void Book::setAvailable(bool status) { isAvailable_ = status; }

// Implementasi Operator Overloading (==) untuk membandingkan ID buku
bool Book::operator==(const Book& other) const {
    return id_ == other.id_;
}

// Implementasi Operator Overloading (<<) untuk mencetak info buku ke terminal
std::ostream& operator<<(std::ostream& os, const Book& book) {
    os << "[" << book.id_ << "] " << book.title_ << " oleh " << book.author_ 
       << " - " << (book.isAvailable_ ? "Tersedia" : "Sedang Dipinjam");
    return os;
}