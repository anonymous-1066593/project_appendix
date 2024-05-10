#ifndef TYPE_GENERATOR_HPP
#define TYPE_GENERATOR_HPP

#include <random>

#include "graph.hpp"
#include "type.hpp"

Type generate_random_type(int max_size, int branching_factor, std::mt19937 &rng, bool recursive = true, int num_participants=2);
Type generate_exponential_counterexample(int k);
Type generate_random_isomorphic_type(int nodes, std::mt19937 &rng);

#endif // TYPE_GENERATOR_HPP
