#ifndef SUBTYPING_HPP
#define SUBTYPING_HPP

#include "type.hpp"

namespace inductive_sub {
    bool subtype(Type &t1, Type &t2, volatile bool &timeout_handler);
}

namespace coinductive_sub {
    bool subtype(Type &t1, Type &t2, volatile bool &timeout_handler);
}

#endif
