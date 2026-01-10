#include "storage.hpp"

ContentStore::Ptr ContentStore::intern(const std::string &content) {
    auto it = map_.find(content);
    if (it != map_.end()) return it->second;
    auto p = std::make_shared<std::string>(content);
    map_.emplace(*p, p);
    return p;
}
