// Arena-style structure to hold a session type, supporting deletion.

#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <string>
#include <map>
#include "graph.hpp"

struct Type {
    graph::GraphNode *root;
    std::vector<graph::GraphNode*> nodes;
    Type() {
        root = nullptr;
    }

    ~Type();

    Type(const Type& other);

    Type& operator=(const Type& other);

    std::wstring to_string();
};

std::map<graph::GraphNode*, graph::GraphNode*> copy_into_type(bool set_root, Type &new_type, const Type &old_type);

#endif // TYPE_HPP
