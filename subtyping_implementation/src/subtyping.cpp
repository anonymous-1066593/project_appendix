#include "type.hpp"
#include "graph.hpp"
#include "subtyping.hpp"

#include <vector>
#include <utility>
#include <unordered_set>

using Node = graph::GraphNode;

int64_t splitmix64(int64_t state) {
    state += 0x9e3779b97f4a7c15;
    int64_t z = state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

namespace inductive_sub {
    struct PairHash {
        std::size_t operator() (const std::pair<Node*, Node*>& p) const {
            int64_t a = reinterpret_cast<int64_t>(p.first);
            int64_t b = reinterpret_cast<int64_t>(p.second);
            return (splitmix64(a) << 32) ^ splitmix64(b);
        }
    };

    using PairSet = std::unordered_set<std::pair<Node*, Node*>, PairHash>;

    bool check_rule(PairSet &sigma, Node *n1, Node *n2, volatile bool &timeout_handler) {
        if(timeout_handler) return false;
        if(sigma.find({n1, n2}) != sigma.end()) { // AS-Assump
            return true;
        }
        if(n1->type() == graph::TypeEnd && n2->type() == graph::TypeEnd) { // AS-End
            return true;
        }
        if(n1->type() == graph::TypeIn && n2->type() == graph::TypeIn) { // AS-In
            auto in1 = static_cast<graph::In*>(n1);
            auto in2 = static_cast<graph::In*>(n2);
            sigma.insert({n1, n2});
            bool result = in1->participant == in2->participant && subsort(in2->payload, in1->payload) && check_rule(sigma, in1->continuation, in2->continuation, timeout_handler);
            sigma.erase({n1, n2});
            return result;
        }
        if(n1->type() == graph::TypeOut && n2->type() == graph::TypeOut) { // AS-Out
            auto out1 = static_cast<graph::Out*>(n1);
            auto out2 = static_cast<graph::Out*>(n2);
            sigma.insert({n1, n2});
            bool result = out1->participant == out2->participant && subsort(out1->payload, out2->payload) && check_rule(sigma, out1->continuation, out2->continuation, timeout_handler);
            sigma.erase({n1, n2});
            return result;
        }
        if(n1->type() == graph::TypeBranch && n2->type() == graph::TypeBranch) { // AS-Branch
            auto branch1 = static_cast<graph::Branch*>(n1);
            auto branch2 = static_cast<graph::Branch*>(n2);
            if(branch1->participant != branch2->participant) return false;
            sigma.insert({n1, n2});
            // Check whether all branches of branch1 are matched by branches of branch2
            size_t branch2_ptr = 0;
            for(size_t i = 0; i < branch1->branches.size(); i++) {
                while(branch2_ptr < branch2->branches.size()
                    && branch2->branches[branch2_ptr].first < branch1->branches[i].first) {
                    branch2_ptr++;
                }
                if(branch2_ptr >= branch2->branches.size()
                    || branch2->branches[branch2_ptr].first != branch1->branches[i].first) { // Not matched
                    return false;
                }
                if(!check_rule(sigma, branch1->branches[i].second, branch2->branches[branch2_ptr].second, timeout_handler)) {
                    return false;
                }
            }
            sigma.erase({n1, n2});
            return true;
        }
        if(n1->type() == graph::TypeSelect && n2->type() == graph::TypeSelect) { // AS-Select
            auto select1 = static_cast<graph::Select*>(n1);
            auto select2 = static_cast<graph::Select*>(n2);
            if(select1->participant != select2->participant) return false;
            sigma.insert({n1, n2});
            // Check whether all branches of select2 are matched by branches of select1
            size_t select1_ptr = 0;
            for(size_t i = 0; i < select2->branches.size(); i++) {
                while(select1_ptr < select1->branches.size()
                    && select1->branches[select1_ptr].first < select2->branches[i].first) {
                    select1_ptr++;
                }
                if(select1_ptr >= select1->branches.size()
                    || select1->branches[select1_ptr].first != select2->branches[i].first) { // Not matched
                    return false;
                }
                if(!check_rule(sigma, select1->branches[select1_ptr].second, select2->branches[i].second, timeout_handler)) {
                    return false;
                }
            }
            sigma.erase({n1, n2});
            return true;
        }
        return false;
    }


    bool subtype(Type &t1, Type &t2, volatile bool &timeout_handler) {
        PairSet sigma;
        return check_rule(sigma, t1.root, t2.root, timeout_handler);
    }
}


namespace coinductive_sub {
    struct PairHash {
        std::size_t operator() (const std::pair<Node*, Node*>& p) const {
            int64_t a = reinterpret_cast<int64_t>(p.first);
            int64_t b = reinterpret_cast<int64_t>(p.second);
            return (splitmix64(a) << 32) ^ splitmix64(b);
        }
    };

    using PairSet = std::unordered_set<std::pair<Node*, Node*>, PairHash>;

    bool check_rule(PairSet &sigma, Node *n1, Node *n2, volatile bool &timeout_handler) {
        if(timeout_handler) return false;
        if(sigma.find({n1, n2}) != sigma.end()) { // AS-Assump
            return true;
        }
        if(n1->type() == graph::TypeEnd && n2->type() == graph::TypeEnd) { // AS-End
            return true;
        }
        if(n1->type() == graph::TypeIn && n2->type() == graph::TypeIn) { // AS-In
            auto in1 = static_cast<graph::In*>(n1);
            auto in2 = static_cast<graph::In*>(n2);
            sigma.insert({n1, n2});
            bool result = in1->participant == in2->participant && subsort(in2->payload, in1->payload) && check_rule(sigma, in1->continuation, in2->continuation, timeout_handler);
            return result;
        }
        if(n1->type() == graph::TypeOut && n2->type() == graph::TypeOut) { // AS-Out
            auto out1 = static_cast<graph::Out*>(n1);
            auto out2 = static_cast<graph::Out*>(n2);
            sigma.insert({n1, n2});
            bool result = out1->participant == out2->participant && subsort(out1->payload, out2->payload) && check_rule(sigma, out1->continuation, out2->continuation, timeout_handler);
            return result;
        }
        if(n1->type() == graph::TypeBranch && n2->type() == graph::TypeBranch) { // AS-Branch
            auto branch1 = static_cast<graph::Branch*>(n1);
            auto branch2 = static_cast<graph::Branch*>(n2);
            if(branch1->participant != branch2->participant) return false;
            sigma.insert({n1, n2});
            // Check whether all branches of branch1 are matched by branches of branch2
            size_t branch2_ptr = 0;
            for(size_t i = 0; i < branch1->branches.size(); i++) {
                while(branch2_ptr < branch2->branches.size()
                    && branch2->branches[branch2_ptr].first < branch1->branches[i].first) {
                    branch2_ptr++;
                }
                if(branch2_ptr >= branch2->branches.size()
                    || branch2->branches[branch2_ptr].first != branch1->branches[i].first) { // Not matched
                    return false;
                }
                if(!check_rule(sigma, branch1->branches[i].second, branch2->branches[branch2_ptr].second, timeout_handler)) {
                    return false;
                }
            }
            return true;
        }
        if(n1->type() == graph::TypeSelect && n2->type() == graph::TypeSelect) { // AS-Select
            auto select1 = static_cast<graph::Select*>(n1);
            auto select2 = static_cast<graph::Select*>(n2);
            if(select1->participant != select2->participant) return false;
            sigma.insert({n1, n2});
            // Check whether all branches of select2 are matched by branches of select1
            size_t select1_ptr = 0;
            for(size_t i = 0; i < select2->branches.size(); i++) {
                while(select1_ptr < select1->branches.size()
                    && select1->branches[select1_ptr].first < select2->branches[i].first) {
                    select1_ptr++;
                }
                if(select1_ptr >= select1->branches.size()
                    || select1->branches[select1_ptr].first != select2->branches[i].first) { // Not matched
                    return false;
                }
                if(!check_rule(sigma, select1->branches[select1_ptr].second, select2->branches[i].second, timeout_handler)) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }


    bool subtype(Type &t1, Type &t2, volatile bool &timeout_handler) {
        PairSet sigma;
        return check_rule(sigma, t1.root, t2.root, timeout_handler);
    }
}

