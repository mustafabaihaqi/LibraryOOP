#pragma once
#include "User.h"
#include <string>

class Member : public User {
private:
    std::string memberId_;
    std::string name_;
    std::string email_;

public:
    Member(std::string username, std::string passwordHash, std::string memberId, std::string name, std::string email);
    std::string getMemberId() const;
    std::string getName() const;
    std::string getEmail() const;

    // Override metode virtual
    std::string role() const override;
    const std::string& getMemberId() const;

    // Prototipe logika peminjaman
    void borrowItem() const;
    void returnItem() const;
};