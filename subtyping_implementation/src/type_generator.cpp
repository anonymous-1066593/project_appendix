#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include "assert.h"

#include "graph.hpp"
#include "type.hpp"
#include "sort.hpp"

Sort random_sort(std::mt19937 &rng) {
    switch(rng() % 3) {
        case 0:
            return Sort::Bool;
        case 1:
            return Sort::Int;
        case 2:
            return Sort::Nat;
    }
    assert(false); // unreachable
}

// Sample size integers, from [lo, hi)
std::vector<int> sample_from_range(std::mt19937 rng, int size, int lo, int hi) {
    std::vector<int> result;
    std::set<int> used;
    for(int i = 0; i < size; i++) {
        int sample;
        do {
            sample = rng() % (hi - lo) + lo;
        } while(used.count(sample) > 0);
        result.push_back(sample);
        used.insert(sample);
    }
    return result;
}

graph::GraphNode* gen_into_type(Type &type, int max_size, int branching_factor, std::mt19937 &rng, bool recursive, std::vector<graph::GraphNode*> &earlier_nodes, int num_participants) {
    auto gen_leaf = [&]() -> graph::GraphNode* {
        bool gen_end = !recursive || earlier_nodes.size() == 0 || (rng() % 2 == 0);
        if(gen_end) {
            graph::End* node = new graph::End();
            type.nodes.push_back(node);
            return node;
        } else {
            return earlier_nodes[rng() % earlier_nodes.size()];
        }
    };

    if(max_size <= 1) {
        return gen_leaf();
    } else {
        switch(rng() % 4) {
            case graph::TypeIn: {
                graph::In* node = new graph::In(rng() % num_participants);
                type.nodes.push_back(node);
                earlier_nodes.push_back(node);
                node->payload = random_sort(rng);
                node->continuation = gen_into_type(type, max_size - 1, branching_factor, rng, recursive, earlier_nodes, num_participants);
                earlier_nodes.pop_back();
                return node;
            }
            case graph::TypeOut: {
                graph::Out* node = new graph::Out(rng() % num_participants);
                earlier_nodes.push_back(node);
                type.nodes.push_back(node);
                node->payload = random_sort(rng);
                node->continuation = gen_into_type(type, max_size - 1, branching_factor, rng, recursive, earlier_nodes, num_participants);
                earlier_nodes.pop_back();
                return node;
            }
            case graph::TypeBranch: {
                graph::Branch* node = new graph::Branch(rng() % num_participants);
                earlier_nodes.push_back(node);
                type.nodes.push_back(node);
                int num_branches = rng() % std::min(max_size - 1, branching_factor) + 1;
                std::vector<int> branch_indices = sample_from_range(rng, num_branches, 0, branching_factor);
                std::vector<int> split_points = sample_from_range(rng, num_branches - 1, 1, max_size - 1);
                std::sort(split_points.begin(), split_points.end());
                split_points.insert(split_points.begin(), 0);
                split_points.push_back(max_size - 1);
                for(int i = 0; i < num_branches; i++) {
                    int branch_idx = branch_indices[i];
                    node->branches.push_back({branch_idx, gen_into_type(type, split_points[i+1] - split_points[i], branching_factor, rng, recursive, earlier_nodes, num_participants)});
                }
                earlier_nodes.pop_back();
                return node;
            }
            case graph::TypeSelect: {
                graph::Select* node = new graph::Select(rng() % num_participants);
                earlier_nodes.push_back(node);
                type.nodes.push_back(node);
                int num_branches = rng() % std::min(max_size - 1, branching_factor) + 1;
                std::vector<int> branch_indices = sample_from_range(rng, num_branches, 0, branching_factor);
                std::vector<int> split_points = sample_from_range(rng, num_branches - 1, 1, max_size - 1);
                std::sort(split_points.begin(), split_points.end());
                split_points.insert(split_points.begin(), 0);
                split_points.push_back(max_size - 1);
                for(int i = 0; i < num_branches; i++) {
                    int branch_idx = branch_indices[i];
                    node->branches.push_back({branch_idx, gen_into_type(type, split_points[i+1] - split_points[i], branching_factor, rng, recursive, earlier_nodes, num_participants)});
                }
                earlier_nodes.pop_back();
                return node;
            }
            // case graph::TypeEnd: {
            //     return gen_leaf();
            // }
            default:
                return nullptr;
        }
    }
}

Type generate_random_type(int max_size, int branching_factor, std::mt19937 &rng, bool recursive = true, int num_participants = 2) {
    Type type;
    std::vector<graph::GraphNode*> earlier_nodes;
    type.root = gen_into_type(type, max_size, branching_factor, rng, recursive, earlier_nodes, num_participants);
    return type;
}

Type generate_exponential_counterexample(int k) {
    Type type;
    graph::Branch* root = new graph::Branch(0);
    type.nodes.push_back(root);
    std::vector<graph::Branch*> main_cycle;
    main_cycle.push_back(root);
    for(int i = 1; i < k; i++) {
        graph::Branch* branch = new graph::Branch(0);
        type.nodes.push_back(branch);
        main_cycle.back()->branches.push_back({1, branch});
        main_cycle.push_back(branch);
    }
    main_cycle.back()->branches.push_back({1, root});

    graph::Branch* sink = new graph::Branch(0);
    type.nodes.push_back(sink);
    sink->branches.push_back({1, sink});
    sink->branches.push_back({2, sink});

    for(int i = 0; i < k; i++) {
        if(i == k-1) {
            main_cycle[i]->branches.push_back({2, root});
        } else {
            graph::Branch* current = new graph::Branch(0);
            type.nodes.push_back(current);
            main_cycle[i]->branches.push_back({2, current});
            for(int j = i+2; j < k; j++) {
                graph::Branch* branch = new graph::Branch(0);
                type.nodes.push_back(branch);
                current->branches.push_back({1, branch});
                current->branches.push_back({2, sink});
                current = branch;
            }
            current->branches.push_back({1, root});
            current->branches.push_back({2, sink});
        }
    }

    type.root = root;
    return type;
}

Type generate_random_isomorphic_type(int nodes, std::mt19937 &rng) {
    // Generate some types coinductively equal to μX. ⊕{l_0: X, l_1: X}
    Type type;
    graph::Branch** root = reinterpret_cast<graph::Branch**>(&type.root);
    std::vector<graph::Branch**> leaves;
    leaves.push_back(root);
    for(int i = 0; i < nodes; i++) {
        graph::Branch* branch = new graph::Branch(0);
        branch->branches.push_back({0, nullptr});
        branch->branches.push_back({1, nullptr});
        type.nodes.push_back(branch);
        int fill_idx = rng() % leaves.size();
        *leaves[fill_idx] = branch;
        leaves.erase(leaves.begin() + fill_idx);
        leaves.push_back(reinterpret_cast<graph::Branch**>(&branch->branches[0].second));
        leaves.push_back(reinterpret_cast<graph::Branch**>(&branch->branches[1].second));
    }
    
    // type.root = *root;
    for(graph::Branch** leaf : leaves) {
        *leaf = *root;
    }
    return type;
}
