#pragma once
#include <string>
#include <memory>
#include <unordered_map>

class ContentStore {
public:
    using Ptr = std::shared_ptr<std::string>;
    Ptr intern(const std::string &content);

private:
    std::unordered_map<std::string, Ptr> map_;
};
