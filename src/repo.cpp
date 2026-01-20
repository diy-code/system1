#include "repo.hpp"
#include "util.hpp"
#include <sstream>
#include <algorithm>
#include <ostream>
//fixed from missing <functional> to include <functional>
#include <functional>

static uint64_t ID_COUNTER = 1;

static std::string simple_id_gen() {
    std::ostringstream oss;
    oss << std::hex << (ID_COUNTER++);
    return oss.str();
}

Repo::Repo() {
    init();
}

void Repo::init() {
    staging_.clear();
    branches_.clear();
    head_ = nullptr;
    head_branch_ = "master";
    branches_[head_branch_] = nullptr;
}

void Repo::add(const std::string &filename, const std::string &content) {
    auto p = store_.intern(content);
    staging_[filename] = p;
}

std::string Repo::make_id(const std::string & /*s*/) const {
    return simple_id_gen();
}

std::shared_ptr<Commit> Repo::make_commit_node(const std::string &message, std::shared_ptr<Commit> parent) {
    auto c = std::make_shared<Commit>();
    if (parent) c->files = parent->files;
    for (auto &kv : staging_) c->files[kv.first] = kv.second;

    c->parent = parent;
    c->message = message;
    c->timestamp = now_timestamp();
    c->id = make_id(message + c->timestamp);
    return c;
}

std::string Repo::commit(const std::string &message) {
    auto c = make_commit_node(message, head_);
    head_ = c;
    branches_[head_branch_] = head_;
    staging_.clear();
    return c->id;
}

std::vector<std::shared_ptr<Commit>> Repo::log() const {
    std::vector<std::shared_ptr<Commit>> out;
    auto cur = head_;
    while (cur) { out.push_back(cur); cur = cur->parent; }
    return out;
}

bool Repo::checkout(const std::string &commit_id) {
    std::function<std::shared_ptr<Commit>(std::shared_ptr<Commit>)> find =
        [&](std::shared_ptr<Commit> node)->std::shared_ptr<Commit> {
            if (!node) return nullptr;
            if (node->id == commit_id) return node;
            return find(node->parent);
        };

    auto found = find(head_);
    if (!found) {
        for (auto &b : branches_) {
            auto n = find(b.second);
            if (n) { found = n; break; }
        }
    }
    if (!found) return false;

    head_ = found;
    branches_[head_branch_] = head_;
    staging_.clear();
    return true;
}

void Repo::branch(const std::string &name) {
    branches_[name] = head_;
}

bool Repo::checkout_branch(const std::string &name) {
    auto it = branches_.find(name);
    if (it == branches_.end()) return false;
    head_branch_ = name;
    head_ = it->second;
    staging_.clear();
    return true;
}

std::vector<std::string> Repo::merge(const std::string &branch_name) {
    std::vector<std::string> conflicts;
    auto it = branches_.find(branch_name);
    if (it == branches_.end()) return conflicts;

    auto other = it->second;
    if (!other) return conflicts;

    std::unordered_map<std::string, std::shared_ptr<Commit>> seen;
    for (auto n = head_; n; n = n->parent) seen[n->id] = n;

    std::shared_ptr<Commit> base = nullptr;
    for (auto n = other; n; n = n->parent) {
        if (seen.count(n->id)) { base = n; break; }
    }

    auto result_files = head_ ? head_->files
                              : std::unordered_map<std::string, std::shared_ptr<std::string>>{};

    for (auto &kv : other->files) {
        const auto &fname = kv.first;
        auto other_content = kv.second;

        auto ours_it = result_files.find(fname);
        std::shared_ptr<std::string> ours_content = (ours_it == result_files.end() ? nullptr : ours_it->second);

        std::shared_ptr<std::string> base_content = nullptr;
        if (base) {
            auto bi = base->files.find(fname);
            if (bi != base->files.end()) base_content = bi->second;
        }

        bool base_eq_ours  = (base_content == ours_content);
        bool base_eq_other = (base_content == other_content);

        if (!base_eq_ours && !base_eq_other && ours_content != other_content) {
            conflicts.push_back(fname);
        } else {
            if (base_eq_ours && other_content) result_files[fname] = other_content;
        }
    }

    if (conflicts.empty()) {
        auto merge_commit = std::make_shared<Commit>();
        merge_commit->parent = head_;
        merge_commit->files = std::move(result_files);
        merge_commit->message = std::string("merge ") + branch_name;
       //fixed from merge_commit->timestamp == now_timestamp(); to merge_commit->timestamp = now_timestamp();
        merge_commit->timestamp = now_timestamp(); 
        merge_commit->id = make_id(merge_commit->message + merge_commit->timestamp);

        head_ = merge_commit;
        branches_[head_branch_] = head_;
    }

    return conflicts;
}

std::vector<std::string> Repo::status() const {
    std::vector<std::string> out;
    for (auto &kv : staging_) out.push_back(std::string("staged: ") + kv.first);
    return out;
}

void Repo::show(std::ostream& os) const {
    print_header(os);
    print_staging_area(os);
    print_branches(os);
    print_all_commits(os);
}


void Repo::print_header(std::ostream& os) const {
    os << "Repository state:\n"
        << "Head branch: " << head_branch_ << "\n"
        << "HEAD commit: ";

    if (head_) {
        os << head_->id << "  (" << head_->timestamp << ")  "<< head_->message << "\n";
    }
    else {
        os << "(none)\n";
    }
}

void Repo::print_staging_area(std::ostream& os) const {
    os << "Staging area:\n";

    if (staging_.empty()) {
        os << "  (empty)\n";
        return;
    }


    for (const auto& [filename, content] : staging_) {
        os << "  " << filename << " -> \"" << *content << "\"\n";
    }
}

void Repo::print_branches(std::ostream& os) const {
    os << "Branches:\n";

    
    //print all branches
    for (const auto& [branch_name, branch_commit] : branches_) {
        os << "  " << branch_name << " -> ";

        if (branch_commit) {
            os << branch_commit->id;
        }
        else {
            os << "(none)";
        }

        if (branch_name == head_branch_) {
            os << "  [HEAD]";
        }
        os << "\n";
    }
}

void Repo::print_all_commits(std::ostream& os) const {
    auto all_commits = collect_all_commits();

    os << "Commits (" << all_commits.size() << "):\n";

    for (const auto& commit : all_commits) {
        print_commit(os, commit);
    }
}

std::vector<std::shared_ptr<Commit>> Repo::collect_all_commits() const {
    std::unordered_map<std::string, std::shared_ptr<Commit>> unique_commits;

    // Collect all reachable commits from all branches
    for (const auto& [_, branch_commit] : branches_) {
        for (auto current = branch_commit; current; current = current->parent) {
            unique_commits[current->id] = current;
        }
    }

    // Convert to vector and return
    std::vector<std::shared_ptr<Commit>> result;
    result.reserve(unique_commits.size());
    for (const auto& [_, commit] : unique_commits) {
        result.push_back(commit);
    }

    return result;
}

void Repo::print_commit(std::ostream& os, const std::shared_ptr<Commit>& commit) const {
    os << "  Commit: " << commit->id << "\n"
        << "    timestamp: " << commit->timestamp << "\n"
        << "    message: " << commit->message << "\n"
        << "    files:\n";

    if (commit->files.empty()) {
        os << "      (none)\n";
        return;
    }

    //print all files in this commit
    for (const auto& [filename, content] : commit->files) {
        os << "      " << filename << " -> \"" << *content << "\"\n";
    }
}

