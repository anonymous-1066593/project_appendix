
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <string>
#include <utility>
#include "sort.hpp"
#include "participant.hpp"

namespace graph {
    using Label = int;

    enum NodeType {
        TypeIn = 0,
        TypeOut = 1,
        TypeBranch = 2,
        TypeSelect = 3,
        TypeEnd = 4,
    };

    // Abstract class for graph node.
    class GraphNode {
        public:
        virtual NodeType type() = 0;

        virtual ~GraphNode() {}

        virtual GraphNode* copy() {
            return nullptr;
        }
    };

    class End : public GraphNode {
        public:
        End() {}

        NodeType type() override { return NodeType::TypeEnd; }

        GraphNode* copy() override {
            return new End();
        }
    };

    class In : public GraphNode {
        public:
        Participant participant;
        Sort payload;
        GraphNode *continuation;

        In(Participant p) {
            participant = p;
        }

        NodeType type() override { return NodeType::TypeIn; }

        GraphNode* copy() override {
            In* new_node = new In(participant);
            new_node->payload = payload;
            new_node->continuation = continuation;
            return new_node;
        }
    };

    class Out : public GraphNode {
        public:
        Participant participant;
        Sort payload;
        GraphNode *continuation;
        
        Out(Participant p) {
            participant = p;
        }
        
        NodeType type() override { return NodeType::TypeOut; }

        GraphNode* copy() override {
            Out* new_node = new Out(participant);
            new_node->payload = payload;
            new_node->continuation = continuation;
            return new_node;
        }
    };

    class Branch : public GraphNode {
        public:
        Participant participant;
        std::vector<std::pair<Label, GraphNode*>> branches; // implicitly: branches must be sorted

        Branch(Participant p) {
            participant = p;
        }

        NodeType type() override { return NodeType::TypeBranch; }

        GraphNode* copy() override {
            Branch* new_node = new Branch(participant);
            for(auto &branch : branches) {
                new_node->branches.push_back({branch.first, branch.second});
            }
            return new_node;
        }
    };

    class Select : public GraphNode {
        public:
        Participant participant;
        std::vector<std::pair<Label, GraphNode*>> branches; // implicitly: branches must be sorted

        Select(Participant p) {
            participant = p;
        }

        NodeType type() override { return NodeType::TypeSelect; }

        GraphNode* copy() override {
            Select* new_node = new Select(participant);
            for(auto &branch : branches) {
                new_node->branches.push_back({branch.first, branch.second});
            }
            return new_node;
        }
    };
}

#endif // GRAPH_HPP
