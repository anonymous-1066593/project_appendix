#include "sort.hpp"

std::wstring to_string(Sort sort) {
    switch(sort) {
        case Int: return L"Int";
        case Nat: return L"Nat";
        case Bool: return L"Bool";
    }
    return L"";
}

bool subsort(Sort s1, Sort s2) {
    return s1 == s2 || (s1 == Nat && s2 == Int);
}
