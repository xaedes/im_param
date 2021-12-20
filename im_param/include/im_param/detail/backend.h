#pragma once

#include <type_traits>
#include <string>

#include "im_param/detail/collection.h"
#include "im_param/detail/cpp11_template_logic.h"
#include "im_param/detail/flags.h"

namespace im_param {

    // cant deduce arguments Args... for dependent types (e.g T<Args...>::type) of template specializations T<Args...>
    // as a workaround use this TypeHolder class which holds the type of the template specialization T<Args...>.
    template <class T, class U=std::false_type> struct TypeHolder {};
    
    template<class T>
    using TypeHolderOf = TypeHolder<typename std::remove_reference<T>::type>;

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
        struct is_base_value : public std::integral_constant<bool,
            disjunction<
                std::is_floating_point<T>,
                is_non_bool_integral<T>,
                std::is_same<T, bool>
            >::value
        >
        {};

        //[[deprecated]]
        template <class T> using is_specialized = is_base_value<T>;
    };

    #pragma region specializations for named parameter values (floats, ints, bools, etc)

    // named parameter multi channel values (floats, ints, bools, etc)
    template<
        typename backend_type,
        typename value_type,
        typename size_type = std::size_t,
        typename... Args,
        std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type* ptr, size_type count, Args... args)
    {
        return backend.parameter(name, ptr, count, std::forward<Args>(args)...);
    }

    // named parameter value (floats, ints, bools, etc)
    template<
        typename backend_type,
        typename value_type,
        typename... Args,
        std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type& value, Args... args)
    {
        return backend.parameter(name, value, std::forward<Args>(args)...);
    }

    #pragma endregion

    #pragma region specializations for named parameter group

    // named parameter group
    template<
        typename backend_type,
        typename value_type,
        typename type_holder_type,
        typename... Args,
        std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type& params, const TypeHolder<type_holder_type>& typeholder, Args... args)
    {
        return backend.parameter(name, params, typeholder, std::forward<Args>(args)...);
    }
    #pragma endregion

    #pragma region specializations for named list of parameter values (floats, ints, bools, etc)
    // named list of parameter multi channel values (floats, ints, bools, etc)
    template<
        typename backend_type,
        typename collection_type,
        typename value_type,
        typename value_iterator_type,
        typename inserter_iterator_type,
        typename callback_type,
        // typename value_type,
        typename size_type = std::size_t,
        typename... Args
        // std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection,
        callback_type callback_get_first_val,
        size_type num_channels, 
        Args... args
    )
    {
        return backend.parameter(
            name, 
            collection, 
            callback_get_first_val, 
            num_channels, 
            std::forward<Args>(args)...
        );
    }

    // named list of parameter values (floats, ints, bools, etc)
    template<
        typename backend_type,
        typename collection_type,
        typename value_type,
        typename value_iterator_type,
        typename inserter_iterator_type,
        typename... Args,
        std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection,
        Args... args
    )
    {
        return backend.parameter(
            name, 
            collection, 
            std::forward<Args>(args)...
        );
    }


    #pragma endregion

    #pragma region specializations for named list of parameter groups
    template<
        typename backend_type,
        typename collection_type,
        typename value_type,
        typename value_iterator_type,
        typename inserter_iterator_type,
        typename type_holder_type,
        typename... Args,
        std::enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        Collection<collection_type,value_type,value_iterator_type,inserter_iterator_type>& collection,
        const TypeHolder<type_holder_type>& typeholder, 
        Args... args
    )
    {
        return backend.parameter(
            name, 
            collection,
            typeholder, 
            std::forward<Args>(args)...
        );
    }
    #pragma endregion


} // namespace
