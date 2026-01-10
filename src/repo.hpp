#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "storage.hpp"

struct Commit {
    std::string id;
    std::string message;
    std::string timestamp;
    std::shared_ptr<Commit> parent;
    std::unordered_map<std::string, std::shared_ptr<std::string>> files;
};

class Repo {
public:
    Repo();
    void init();
    void add(const std::string &filename, const std::string &content);
    std::string commit(const std::string &message);
    std::vector<std::shared_ptr<Commit>> log() const;
    bool checkout(const std::string &commit_id);

    void branch(const std::string &name);
    bool checkout_branch(const std::string &name);

    std::vector<std::string> merge(const std::string &branch_name);
    std::vector<std::string> status() const;

    std::shared_ptr<Commit> head() const { return head_; }

private:
    std::shared_ptr<Commit> make_commit_node(const std::string &message, std::shared_ptr<Commit> parent);
    std::string make_id(const std::string &s) const;

private:
    ContentStore store_;
    std::unordered_map<std::string, std::shared_ptr<std::string>> staging_;
    std::unordered_map<std::string, std::shared_ptr<Commit>> branches_;
    std::shared_ptr<Commit> head_;
    std::string head_branch_;
};
