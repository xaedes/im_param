#pragma once

#include <type_traits>

namespace im_param {

    #pragma region c++11 implementations of c++14 template functions
    template<bool C, class A, class B>
    using conditional_t = typename std::conditional<C,A,B>::type;

    template<bool C, class T>
    using enable_if_t = typename std::enable_if<C,T>::type;
    #pragma endregion

    #pragma region c++11 implementations of c++17 template logic functions
    // https://en.cppreference.com/w/cpp/types/conjunction
    template<class...> struct conjunction : std::true_type { };
    template<class B1> struct conjunction<B1> : B1 { };
    template<class B1, class... Bn>
    struct conjunction<B1, Bn...> 
        : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

    // https://en.cppreference.com/w/cpp/types/disjunction
    template<class...> struct disjunction : std::false_type { };
    template<class B1> struct disjunction<B1> : B1 { };
    template<class B1, class... Bn>
    struct disjunction<B1, Bn...> 
        : conditional_t<bool(B1::value), B1, disjunction<Bn...>>  { };

    // https://en.cppreference.com/w/cpp/types/negation
    template<class B>
    struct negation : std::integral_constant<bool, !bool(B::value)> { };
    #pragma endregion

} // namespace
