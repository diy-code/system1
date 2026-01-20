#include "repo.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

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
    assert(conf.has_value());  // First check merge succeeded (branch was found)
    assert(conf->empty());     // Then check no conflicts
    assert(r.head() != nullptr);
    assert(!r.head()->timestamp.empty()); 

    assert(r.head()->files.count("y.txt")==1);

    std::cout << "all tests passed\n";

    // Additional tests for show() function
    std::cout << "\n=== Testing show() function ===\n";
    
    // Test 1: show() on empty repository
    std::cout << "\nTest 1: Empty repository\n";
    Repo r1;
    r1.init();
    std::ostringstream oss1;
    r1.show(oss1);
    std::string output1 = oss1.str();
    assert(output1.find("Repository state:") != std::string::npos);
    assert(output1.find("HEAD commit: (none)") != std::string::npos);
    assert(output1.find("Staging area:") != std::string::npos);
    assert(output1.find("(empty)") != std::string::npos);
    std::cout << "   Empty repo shows correctly\n";
    
    // Test 2: show() with items in staging area
    std::cout << "\nTest 2: Repository with staging area\n";
    Repo r2;
    r2.init();
    r2.add("file1.txt", "content1");
    r2.add("file2.txt", "content2");
    std::ostringstream oss2;
    r2.show(oss2);
    std::string output2 = oss2.str();
    assert(output2.find("Staging area:") != std::string::npos);
    assert(output2.find("file1.txt") != std::string::npos);
    assert(output2.find("file2.txt") != std::string::npos);
    assert(output2.find("content1") != std::string::npos);
    assert(output2.find("content2") != std::string::npos);
    std::cout << "   Staging area displayed correctly\n";
    
    // Test 3: show() after commits
    std::cout << "\nTest 3: Repository with commits\n";
    Repo r3;
    r3.init();
    r3.add("test.txt", "version1");
    std::string commit1_id = r3.commit("first commit");
    r3.add("test.txt", "version2");
    std::string commit2_id = r3.commit("second commit");
    std::ostringstream oss3;
    r3.show(oss3);
    std::string output3 = oss3.str();
    assert(output3.find("HEAD commit:") != std::string::npos);
    assert(output3.find(commit2_id) != std::string::npos);
    assert(output3.find("second commit") != std::string::npos);
    assert(output3.find("Commits (2)") != std::string::npos);
    std::cout << "   Commits displayed correctly\n";
    
    // Test 4: show() with multiple branches
    std::cout << "\nTest 4: Repository with multiple branches\n";
    Repo r4;
    r4.init();
    r4.add("main.txt", "main content");
    r4.commit("initial");
    r4.branch("feature1");
    r4.branch("feature2");
    r4.branch("bugfix");
    std::ostringstream oss4;
    r4.show(oss4);
    std::string output4 = oss4.str();
    assert(output4.find("Branches:") != std::string::npos);
    assert(output4.find("master") != std::string::npos);
    assert(output4.find("feature1") != std::string::npos);
    assert(output4.find("feature2") != std::string::npos);
    assert(output4.find("bugfix") != std::string::npos);
    assert(output4.find("[HEAD]") != std::string::npos);
    std::cout << "   Multiple branches displayed correctly\n";
    
    // Test 5: show() displays file contents in commits
    std::cout << "\nTest 5: Commit file contents\n";
    Repo r5;
    r5.init();
    r5.add("app.cpp", "main code");
    r5.add("util.hpp", "utilities");
    r5.commit("add files");
    std::ostringstream oss5;
    r5.show(oss5);
    std::string output5 = oss5.str();
    assert(output5.find("files:") != std::string::npos);
    assert(output5.find("app.cpp") != std::string::npos);
    assert(output5.find("util.hpp") != std::string::npos);
    assert(output5.find("main code") != std::string::npos);
    assert(output5.find("utilities") != std::string::npos);
    std::cout << "   Commit file contents displayed correctly\n";
    
    // Test 6: show() after branch checkout
    std::cout << "\nTest 6: After branch checkout\n";
    Repo r6;
    r6.init();
    r6.add("data.txt", "v1");
    r6.commit("base commit");
    r6.branch("dev");
    r6.checkout_branch("dev");
    std::ostringstream oss6;
    r6.show(oss6);
    std::string output6 = oss6.str();
    assert(output6.find("Head branch: dev") != std::string::npos);
    assert(output6.find("dev") != std::string::npos);
    assert(output6.find("[HEAD]") != std::string::npos);
    std::cout << "   Branch checkout reflected correctly\n";
    
    std::cout << "\n=== All show() tests passed! ===\n";
    
    return 0;
}
