
#ifndef SORT_HPP
#define SORT_HPP

#include <string>

enum Sort {
    Int = 0,
    Nat = 1,
    Bool = 2,
};

std::wstring to_string(Sort sort);
bool subsort(Sort s1, Sort s2);

#endif
