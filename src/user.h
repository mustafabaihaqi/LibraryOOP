#pragma once
#include <string>

class User {
protected:
    std::string username_;
    std::string passwordHash_;

public:
    User(std::string username, std::string passwordHash);
    virtual ~User() = default;

    bool login(const std::string& inputPassword) const;
    
    virtual std::string role() const = 0; 

    const std::string& getUsername() const;
};