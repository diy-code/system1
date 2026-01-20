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

void Repo::show(std::ostream &os) const {
    // TODO: print the full internal structure
    //=============== Section 1: Repository State ===============
    os << "Repository state:" << std::endl;

    //=============== Section 2: Head Branch ===============
    os<< "Head branch: " << head_branch_ << std::endl;

    //=============== Section 3: Head Commit Summery ===============
    if(head_) {
        os << "HEAD commit: " << head_->id << "  (" << head_->timestamp << ")  " << head_->message << std::endl;
    }
    else {
        os << "HEAD commit: (none)" << std::endl;
    }

    //=============== Section 4: Staging Area ===============
    os << "Staging area: " << std::endl;
    if(staging_.empty()) {
        os << "  (empty)" << std::endl;
    }
    else {
        for(auto &kv : staging_) {
            os << "  " << kv.first << " -> \"" << *kv.second << "\"\n";        }
    }

    //=============== Section 5: Branches ===============
    os <<"Branches:" << std::endl;
    for(const auto &kv : branches_) {
        const std::string &branch_name = kv.first;
        const auto &branch_commit = kv.second;

        os << "  "<<branch_name <<" -> ";

        if(branch_commit) {
            os << branch_commit->id;
        }
        else {
            os << "(none)";
        }
        
        //Mark current branch with [HEAD]
        if(branch_name == head_branch_) {
            os << "  [HEAD]";
        }
        os << std::endl;
    }


    //=============== Section 6: All Commits ===============
    //first collect all unique commits from all branches
    std::unordered_map<std::string, std::shared_ptr<Commit>> all_commits;
    //walk from each branch to collect all reachable commits
    for( const auto &branch_kv : branches_) {
        auto current = branch_kv.second;
        while(current) {
            //Use the commit ID as a key to avoid duplicates
            all_commits[current->id] = current;
            current = current->parent;
        }
    }
    // Print Header with total commits
    os <<"Commits " <<"(" << all_commits.size() << "):" << std::endl;

    for(const auto &commit_kv : all_commits) {
        const auto &commit = commit_kv.second;

        os << "  Commit: " << commit->id<< std::endl;
        os << "    timestamp: "<< commit->timestamp << std::endl;
        os << "    message: "<< commit->message << std::endl;
        os << "    files:\n";

        //print all files in this commit
        if(commit->files.empty()) {
            os << "      (none)" << std::endl;
        } else{
            for(const auto &file_kv : commit->files) {
                const std::string &filename = file_kv.first;
                const auto &content_ptr = file_kv.second;
                
                os <<"      " << filename << " -> \"" << *content_ptr << "\"" << std::endl;
            }
        }

    }

}