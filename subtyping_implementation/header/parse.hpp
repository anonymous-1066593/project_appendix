#ifndef PARSE_HPP
#define PARSE_HPP

#include "ast.hpp"
#include "type.hpp"

Type parse_ast(ast::ASTNode *ast_node);

#endif
