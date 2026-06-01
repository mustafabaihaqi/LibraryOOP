#pragma once
#include <string>
#include <iostream>

class Book {
private:
    int id_;
    std::string title_;
    std::string author_;
    bool isAvailable_;

public:
    Book(int id, std::string title, std::string author);

    // Getters dan Setters (Enkapsulasi)
    int getId() const;
    const std::string& getTitle() const;
    const std::string& getAuthor() const;
    bool isAvailable() const;
    void setAvailable(bool status);

    // Operator overloading
    bool operator==(const Book& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Book& book);
};