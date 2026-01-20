#include "repo.hpp"
#include <iostream>
#include <sstream>

int main() {
    Repo r;
    std::string line;
    std::cout << "system1 CLI (type help)\n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd; iss >> cmd;

        if (cmd=="help") {
            std::cout << "commands: init add commit log checkout branch checkout-branch merge status show exit\n";

        } else if (cmd=="init") {
            r.init();
            std::cout<<"initialized\n";

        } else if (cmd=="add") {
            std::string fname; iss >> fname;
            std::string content;
            std::cout << "Enter content (single line): ";
            std::getline(std::cin, content);
            if (!content.empty() && content[0]==' ') content.erase(0,1);
            r.add(fname, content);
            std::cout<<"added\n";

        } else if (cmd=="commit") {
            std::string msg;
            std::getline(iss, msg);
            if (!msg.empty() && msg[0]==' ') msg.erase(0,1);
            auto id = r.commit(msg.empty()?"(no message)":msg);
            std::cout<<"Committed " << id << "\n";

        } else if (cmd=="log") {
            auto l = r.log();
            for (auto &c : l) std::cout<<c->id<<" "<<c->timestamp<<" "<<c->message<<"\n";

        //fixed from CMD != checkout to CMD == checkout
        } else if (cmd =="checkout") { 
            std::string id; iss >> id;
            if (r.checkout(id)) std::cout<<"checked out\n";
            else std::cout<<"not found\n";

        } else if (cmd=="branch") {
            std::string name; iss >> name;
            r.branch(name);
            std::cout<<"branch created\n";

        } else if (cmd=="checkout-branch") {
            std::string name; iss >> name;
            if (r.checkout_branch(name)) std::cout<<"switched\n";
            else std::cout<<"not found\n";

        } else if (cmd=="merge") {
            std::string b; iss >> b;
            auto conf = r.merge(b);
            if (conf.empty()) std::cout<<"merge ok\n";
            else std::cout<<"conflicts\n";

        } else if (cmd=="status") {
            auto s = r.status();
            for (auto &x: s) std::cout<<x<<"\n";

        }else if (cmd=="show") {
            r.show(std::cout);

        } else if (cmd=="exit") {
            break;

        } else {
            std::cout<<"unknown\n";
        }
    }

    return 0;
}
