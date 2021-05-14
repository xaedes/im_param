#pragma once

#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/flags.h"

namespace im_param {

    struct Backend {
        template <class T>
        struct is_non_bool_integral : public std::integral_constant<bool,
            conjunction<
                std::is_integral<T>,
                negation<std::is_same<T, bool>>
            >::value
        >
        {};

        template <class T>
        struct is_specialized : public std::integral_constant<bool,
            disjunction<
                std::is_floating_point<T>,
                is_non_bool_integral<T>,
                std::is_same<T, bool>
            >::value
        >
        {};
    };

    #pragma region specializations for named parameters (floats, ints, bools, etc)

    // single value
    template<
        typename backend_type,
        typename value_type,
        typename... Args,
        std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type& value, Args... args)
    {
        return backend.parameter(name, value, std::forward<Args>(args)...);
    }

    // multiple values
    template<
        typename backend_type,
        typename value_type,
        typename size_type = std::size_t,
        typename... Args,
        std::enable_if_t<Backend::is_specialized<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type* ptr, size_type count, Args... args)
    {
        return backend.parameter(name, ptr, count, std::forward<Args>(args)...);
    }

    #pragma endregion

    #pragma region specializations for named parameter group

    // group of values
    template<
        typename backend_type,
        typename value_type,
        typename type_holder_type,
        typename... Args,
        std::enable_if_t<!Backend::is_specialized<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type& params, const type_holder_type& typeholder, Args... args)
    {
        return backend.parameter(name, params, typeholder, std::forward<Args>(args)...);
    }
    #pragma endregion

    // cant deduce arguments Args... for dependent types (e.g T<Args...>::type) of template specializations T<Args...>
    // as a workaround use this TypeHolder class which holds the type of the template specialization T<Args...>.
    template <class T> struct TypeHolder {};
    
} // namespace
