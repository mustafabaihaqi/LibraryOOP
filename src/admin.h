#pragma once
#include "user.h"
#include <string>

class Admin : public User {
public:
    Admin(std::string username, std::string passwordHash);

    // Override metode virtual
    std::string role() const override;

    // Prototipe logika
    void addBook() const;
    void deleteBook() const;
    void listMembers() const;
};