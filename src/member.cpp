#include "member.h"

Member::Member(std::string id, std::string name, std::string email)
    : id_(id), name_(name), email_(email) {}

std::string Member::getId() const { return id_; }
std::string Member::getName() const { return name_; }
std::string Member::getEmail() const { return email_; }