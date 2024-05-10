#include <map>
#include <queue>

#include "type.hpp"
#include "graph.hpp"

using namespace graph;

Type unfold_once(Type &old_type) {
    // Unfolding redirects all incoming edges into the start state to a copy of the type
    Type new_type;
    // Copy over to new type

    std::map<GraphNode*, GraphNode*> original_mapping = copy_into_type(true, new_type, old_type);
    
    auto redirect_node_ptr = [&](GraphNode* &node) {
        auto new_mapping = copy_into_type(false, new_type, old_type);
        node = new_mapping[old_type.root];
    };
    
    for(GraphNode *old_node : old_type.nodes) {
        GraphNode *node = original_mapping[old_node];
        switch(node->type()) {
            case NodeType::TypeIn: {
                In* in_node = static_cast<In*>(node);
                if(in_node->continuation == new_type.root) {
                    redirect_node_ptr(in_node->continuation);
                }
                break;
            }
            case NodeType::TypeOut: {
                Out* out_node = static_cast<Out*>(node);
                if(out_node->continuation == new_type.root) {
                    redirect_node_ptr(out_node->continuation);
                }
                break;
            }
            case NodeType::TypeBranch: {
                Branch* branch_node = static_cast<Branch*>(node);
                for (auto &branch : branch_node->branches) {
                    if(branch.second == new_type.root) {
                        redirect_node_ptr(branch.second);
                    }
                }
                break;
            }
            case NodeType::TypeSelect: {
                Select* select_node = static_cast<Select*>(node);
                for (auto &branch : select_node->branches) {
                    if(branch.second == new_type.root) {
                        redirect_node_ptr(branch.second);
                    }
                }
                break;
            }
            case NodeType::TypeEnd:
                break;
            default:
                break;
        }
    }

    return new_type;
}
