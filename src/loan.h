#pragma once
#include <string>

class Loan {
private:
    int loanId_;
    int bookId_;         // Foreign key ke Book
    std::string memberId_; // Foreign key ke Member
    std::string dueDate_;

public:
    Loan(int loanId, int bookId, std::string memberId, std::string dueDate);
    
    int getLoanId() const;
    int getBookId() const;
    const std::string& getMemberId() const;
    const std::string& getDueDate() const;

    bool isOverdue(const std::string& currentDate) const;
};