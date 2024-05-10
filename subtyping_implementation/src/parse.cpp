#include "ast.hpp"
#include "type.hpp"
#include "graph.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include "assert.h"

// Helper for parse_ast

graph::GraphNode* parse_ast_node(
    ast::ASTNode *ast_node,
    std::map<ast::TVar, graph::GraphNode*> node_map,
    std::vector<ast::TVar> incoming_mu,
    Type &t
) {
    auto register_node = [&](graph::GraphNode* new_node) {
        for(ast::TVar var : incoming_mu) {
            node_map[var] = new_node;
        }
        incoming_mu.clear();
        t.nodes.push_back(new_node);
    };
    switch(ast_node->type()) {
        case ast::NodeType::TypeEnd: {
            graph::End* end_node = new graph::End();
            t.nodes.push_back(end_node);
            return end_node;
        }
        case ast::NodeType::TypeIn: {
            ast::In* in = dynamic_cast<ast::In*>(ast_node);
            graph::In* in_node = new graph::In(in->participant);
            register_node(in_node);
            in_node->payload = in->payload;
            in_node->continuation = parse_ast_node(in->continuation, node_map, incoming_mu, t);
            return in_node;
        }
        case ast::NodeType::TypeOut: {
            ast::Out* out = dynamic_cast<ast::Out*>(ast_node);
            graph::Out* out_node = new graph::Out(out->participant);
            register_node(out_node);
            out_node->payload = out->payload;
            out_node->continuation = parse_ast_node(out->continuation, node_map, incoming_mu, t);
            return out_node;
        }
        case ast::NodeType::TypeBranch: {
            ast::Branch* branch = dynamic_cast<ast::Branch*>(ast_node);
            graph::Branch* branch_node = new graph::Branch(branch->participant);
            register_node(branch_node);
            for(auto &pair : branch->branches) {
                branch_node->branches.push_back(std::make_pair(pair.first, parse_ast_node(pair.second, node_map, incoming_mu, t)));
            }
            return branch_node;
        }
        case ast::NodeType::TypeSelect: {
            ast::Select* select = dynamic_cast<ast::Select*>(ast_node);
            graph::Select* select_node = new graph::Select(select->participant);
            register_node(select_node);
            for(auto &pair : select->branches) {
                select_node->branches.push_back(std::make_pair(pair.first, parse_ast_node(pair.second, node_map, incoming_mu, t)));
            }
            return select_node;
        }
        case ast::NodeType::TypeMu: {
            ast::Mu* mu = dynamic_cast<ast::Mu*>(ast_node);
            incoming_mu.push_back(mu->var);
            return parse_ast_node(mu->body, node_map, incoming_mu, t);
        }
        case ast::NodeType::TypeVar: {
            ast::Var* var = dynamic_cast<ast::Var*>(ast_node);
            assert(find(incoming_mu.begin(), incoming_mu.end(), var->var) == incoming_mu.end());
            assert(node_map.find(var->var) != node_map.end());
            return node_map[var->var];
        }
        default:
            assert(false);
    }
}

Type parse_ast(ast::ASTNode *ast_node) {
    std::map<ast::TVar, graph::GraphNode*> node_map;
    std::vector<ast::TVar> incoming_mu;
    Type t;

    graph::GraphNode* root = parse_ast_node(ast_node, node_map, incoming_mu, t);
    t.root = root;
    return t;
}
