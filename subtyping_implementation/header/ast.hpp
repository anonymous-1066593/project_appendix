#ifndef AST_HPP
#define AST_HPP

#include <vector>

#include "participant.hpp"
#include "sort.hpp"

namespace ast {
    using Label = int;
    using TVar = int;

    enum NodeType {
        TypeIn = 0,
        TypeOut = 1,
        TypeBranch = 2,
        TypeSelect = 3,
        TypeEnd = 4,
        TypeMu = 5,
        TypeVar = 6,
    };

    class ASTNode {
        public:
        virtual NodeType type() = 0;
    };

    class End : public ASTNode {
        public:
        NodeType type() override { return NodeType::TypeEnd; }

        End() {}
    };

    class In : public ASTNode {
        public:
        Participant participant;
        Sort payload;
        ASTNode *continuation;

        NodeType type() override { return NodeType::TypeIn; }

        In(Participant participant, Sort payload, ASTNode *continuation)
            : participant(participant), payload(payload), continuation(continuation) {}
    };

    class Out : public ASTNode {
        public:
        Participant participant;
        Sort payload;
        ASTNode *continuation;

        NodeType type() override { return NodeType::TypeOut; }

        Out(Participant participant, Sort payload, ASTNode *continuation)
            : participant(participant), payload(payload), continuation(continuation) {}
    };

    class Branch : public ASTNode {
        public:
        Participant participant;
        std::vector<std::pair<Label, ASTNode*>> branches; // implicitly: branches must be sorted

        NodeType type() override { return NodeType::TypeBranch; }

        Branch(Participant participant, std::vector<std::pair<Label, ASTNode*>> branches)
            : participant(participant), branches(branches) {}
    };

    class Select : public ASTNode {
        public:
        Participant participant;
        std::vector<std::pair<Label, ASTNode*>> branches; // implicitly: branches must be sorted
        
        NodeType type() override { return NodeType::TypeSelect; }

        Select(Participant participant, std::vector<std::pair<Label, ASTNode*>> branches)
            : participant(participant), branches(branches) {}
    };

    class Mu : public ASTNode {
        public:
        TVar var;
        ASTNode *body;

        NodeType type() override { return NodeType::TypeMu; }

        Mu(TVar var, ASTNode *body)
            : var(var), body(body) {}
    };

    class Var : public ASTNode {
        public:
        TVar var;

        NodeType type() override { return NodeType::TypeVar; }

        Var(TVar var)
            : var(var) {}
    };
}

#endif
