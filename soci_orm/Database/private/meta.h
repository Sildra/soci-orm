#pragma once

#include <type_traits>

namespace soci_orm
{
namespace meta
{
template<typename T>
struct is_basic_string {
    template<typename A>
    static constexpr bool test(A* pt, decltype(pt->c_str())* = nullptr) {
        return true;
    }
    template<typename A>
    static constexpr bool test(...) { return false; }
    static constexpr bool value = test<typename std::decay<T>::type>(nullptr);
};
template<typename T>
static constexpr bool is_basic_string_v = is_basic_string<T>::value;

template<typename T>
struct is_collection {
    template<typename A>
    static constexpr bool test(A* pt, typename A::iterator* = nullptr) {
        return true;
    }
    template<typename A>
    static constexpr bool test(...) { return false; }
    static constexpr bool value = test<typename std::decay<T>::type>(nullptr);
};
template<typename T>
static constexpr bool is_collection_v = is_collection<T>::value;

template<typename T>
struct is_pair {
    template<typename A>
    static constexpr bool test(A* pt, typename A::first_type* = nullptr) {
        return true;
    }
    template<typename A>
    static constexpr bool test(...) { return false; }
    static constexpr bool value = test<typename std::decay<T>::type>(nullptr);
};
template<typename T>
static constexpr bool is_pair_v = is_pair<T>::value;
} /* !namespace meta */
} /* !namespace soci_orm */