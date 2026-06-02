#pragma once
#include <vector>
#include "Database.h"

template <typename T>
class Repository {
public:
    virtual T findById(int id) = 0;
    virtual void save(const T& obj) = 0;
    virtual void remove(int id) = 0;
    virtual std::vector<T> listAll() = 0;
    virtual ~Repository() = default;
};

// Implementasi spesifik untuk entitas Book
#include "Book.h"

class BookRepository : public Repository<Book> {
private:
    Database& db_; 

public:
    BookRepository(Database& db) : db_(db) {}

    Book findById(int id) override;
    void save(const Book& obj) override;
    void remove(int id) override;
    std::vector<Book> listAll() override;
};

#include "member.h"

class MemberRepository {
private:
    Database& db_;
    
public:
    MemberRepository(Database& db) : db_(db) {}
    void save(const Member& obj);
    Member findById(const std::string& id);
    std::vector<Member> listAll();
    void remove(const std::string& id);
};

struct LoanRecord {
    int loanId;
    std::string bookTitle;
    std::string memberName;
    std::string dueDate;
};

class LoanRepository {
private:
    Database& db_;
public:
    LoanRepository(Database& db) : db_(db) {}
    
    void issueLoan(int bookId, const std::string& memberId);
    void returnLoan(int bookId);
    std::vector<LoanRecord> listActiveLoans();
};