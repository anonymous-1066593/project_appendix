#include "type.hpp"
#include "graph.hpp"
#include "sort.hpp"

#include <sstream>
#include <set>
#include <map>
#include <string>

using namespace graph;

std::map<GraphNode*, GraphNode*> copy_into_type(bool set_root, Type &new_type, const Type &old_type) {
    std::map<GraphNode*, GraphNode*> mapping;

    for (auto &node : old_type.nodes) {
        GraphNode* new_node = node->copy();
        new_type.nodes.push_back(new_node);
        mapping[node] = new_node;
    }

    for (GraphNode *old_node : old_type.nodes) {
        GraphNode *node = mapping[old_node];
        switch(node->type()) {
            case NodeType::TypeIn: {
                In* in_node = static_cast<In*>(node);
                in_node->continuation = mapping[static_cast<In*>(node)->continuation];
                break;
            }
            case NodeType::TypeOut: {
                Out* out_node = static_cast<Out*>(node);
                out_node->continuation = mapping[static_cast<Out*>(node)->continuation];
                break;
            }
            case NodeType::TypeBranch: {
                Branch* branch_node = static_cast<Branch*>(node);
                for (auto &branch : branch_node->branches) {
                    branch = {branch.first, mapping[branch.second]};
                }
                break;
            }
            case NodeType::TypeSelect: {
                Select* select_node = static_cast<Select*>(node);
                for (auto &branch : select_node->branches) {
                    branch = {branch.first, mapping[branch.second]};
                }
                break;
            }
            case NodeType::TypeEnd:
                break;
            default:
                break;
        }
    }

    if(set_root) {
        new_type.root = mapping[old_type.root];
    }
    return mapping;
};

Type::~Type(){
    for(auto node : nodes) {
        delete node;
    }
}

Type::Type(const Type& other) {
    copy_into_type(true, *this, other);
}

Type& Type::operator=(const Type& other) {
    for(auto node : nodes) {
        delete node;
    }
    nodes.clear();
    copy_into_type(true, *this, other);
    return *this;
}

std::wstring variables = L"XYZWVUTSRQPONMLKJIHGFEDCBA";

std::wstring print(graph::GraphNode* node, std::set<graph::GraphNode*> &seen_nodes, std::map<graph::GraphNode*, wchar_t> &node_vars, int &next_var) {
    if(seen_nodes.find(node) != seen_nodes.end()) {
        if(node_vars.find(node) == node_vars.end()) {
            node_vars[node] = variables[(next_var++) % variables.size()];
        }
        return std::wstring(1, node_vars[node]); // Return the variable name
    }
    std::wstring output = L"";
    seen_nodes.insert(node);
    switch(node->type()) {
        case graph::TypeIn: {
            graph::In* in = static_cast<graph::In*>(node);
            output += L"p";
            output += std::to_wstring(in->participant);
            output += L"?[";
            output += to_string(in->payload);
            output += L"];";
            output += print(in->continuation, seen_nodes, node_vars, next_var);
            break;
        }
        case graph::TypeOut: {
            graph::Out* out = static_cast<graph::Out*>(node);
            output += L"p";
            output += std::to_wstring(out->participant);
            output += L"![";
            output += to_string(out->payload);
            output += L"];";
            output += print(out->continuation, seen_nodes, node_vars, next_var);
            break;
        }
        case graph::TypeSelect: {
            graph::Select* select = static_cast<graph::Select*>(node);
            output += L"p";
            output += std::to_wstring(select->participant);
            output += L"&{";
            bool first_branch = true;
            for(auto branch : select->branches) {
                if(!first_branch) {
                    output += L", ";
                }
                output += L"l" + std::to_wstring(branch.first) + L": " + print(branch.second, seen_nodes, node_vars, next_var);
                first_branch = false;
            }
            output += L"}";
            break;
        }
        case graph::TypeBranch: {
            graph::Branch* branch = static_cast<graph::Branch*>(node);
            output += L"p";
            output += std::to_wstring(branch->participant);
            output += L"\u2295{";
            bool first_branch = true;
            for(auto branch : branch->branches) {
                if(!first_branch) {
                    output += L", ";
                }
                output += L"l" + std::to_wstring(branch.first) + L": " + print(branch.second, seen_nodes, node_vars, next_var);
                first_branch = false;
            }
            output += L"}";
            break;
        }
        case graph::TypeEnd: {
            output += L"end";
            break;
        }
    }
    // Insert a mu if necessary
    if(node_vars.find(node) != node_vars.end()) {
        output = L"\u03bc" + std::wstring(1, node_vars[node]) + L"." + output;
    }
    seen_nodes.erase(node);
    return output;
}

std::wstring Type::to_string() {
    auto seen_nodes = std::set<graph::GraphNode*>();
    auto node_vars = std::map<graph::GraphNode*, wchar_t>();
    int next_var = 0;
    return print(root, seen_nodes, node_vars, next_var);
}
