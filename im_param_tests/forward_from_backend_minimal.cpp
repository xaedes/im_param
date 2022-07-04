

#include "im_param/detail/cpp11_template_logic.h"

#include "tests_common.h"

namespace im_param {

//     // forward declaration for custom types

//     template<class backend_type, class T, class U, class... Args>
//     inline backend_type& parameter(
//         backend_type& backend,
//         T& params, 
//         const TypeHolder<U>&,
//         Args... args
//     );

    template <class T, class U=std::false_type> struct TypeHolder {};

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
                std::is_same<T, bool>,
                std::is_same<T, std::string>
            >::value
        >
        {};

        template <class T>
        struct is_type_holder : public std::integral_constant<bool, false> {};
        
        template <class T, class U>
        struct is_type_holder<TypeHolder<T,U>> : public std::integral_constant<bool, true> {};

    };

    template<
        typename backend_type,
        typename value_type,
        typename type_holder_type,
        typename... Args,
        enable_if_t<!Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(backend_type& backend, const std::string& name, value_type& params, const TypeHolder<type_holder_type>& typeholder, Args... args)
    {
        return backend.parameter(name, params, typeholder, std::forward<Args>(args)...);
    }

} // namespace im_param

struct Backend
{
    int counter = 0;

    template<typename T, typename U, class... Args, im_param::enable_if_t<!im_param::Backend::is_base_value<T>::value, bool> = true>
    Backend& parameter(const std::string& name, T& params, const U& typeholder, Args... args)
    {
        ++counter;
        im_param::parameter(*this, params, typeholder, std::forward<Args>(args)...);
        return *this;
    }

};

struct SimpleFoo
{
    int counter = 0;
};


namespace im_param {

    // describe struct ::SimpleFoo

    template<
        class backend_type, class T, class U, 
        enable_if_t<std::is_same<T, ::SimpleFoo>::value && std::is_same<U, ::SimpleFoo>::value, bool> = true,
        class... Args
    >
    backend_type& parameter(
        backend_type& backend,
        T& params, 
        const TypeHolder<U>&,
        Args... args)
    {
        ++params.counter;
        return backend;
    }

} // namespace im_param

int forward_from_backend_minimal(int argc, char* argv[])
{
    {
        Backend backend;
        SimpleFoo simple;
        ASSERT(simple.counter == 0);
        im_param::parameter(backend, "simple", simple, im_param::TypeHolder<SimpleFoo>());
        ASSERT(simple.counter == 1);
    }

    return 0;
}
