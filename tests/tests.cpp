#include "repo.hpp"
#include <cassert>
#include <iostream>

int main() {
    Repo r;
    r.init();

    r.add("x.txt", "v1");
    auto base = r.commit("base");
    assert(!base.empty());
    assert(r.head() && r.head()->files.count("x.txt")==1);

    r.branch("dev");
    assert(r.checkout_branch("dev"));
    r.add("y.txt", "dev-only");
    r.commit("dev change");

    assert(r.checkout_branch("master"));

    auto conf = r.merge("dev");
    assert(conf.empty());
    assert(r.head() != nullptr);
    assert(!r.head()->timestamp.empty()); 

    assert(r.head()->files.count("y.txt")==1);

    std::cout << "all tests passed\n";
    return 0;
}
