#ifndef MEMBER_H
#define MEMBER_H
#include <string>

class Member {
private:
    std::string id_;
    std::string name_;
    std::string email_;

public:
    Member(std::string id, std::string name, std::string email);
    std::string getId() const;
    std::string getName() const;
    std::string getEmail() const;
};

#endif