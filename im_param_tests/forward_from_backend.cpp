

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

namespace im_param {

    // forward declaration for custom types

    template<class backend_type, class T, class U, class... Args>
    inline backend_type& parameter(
        backend_type& backend,
        T& params, 
        const TypeHolder<U>&,
        Args... args
    );

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

struct SimpleBar
{
    int counter = 0;
};

namespace im_param {

    // describe struct ::SimpleFoo

    template<class backend_type, class T=::SimpleFoo, class U=::SimpleFoo, class... Args>
    inline backend_type& parameter(
        backend_type& backend,
        ::SimpleFoo& params, 
        const TypeHolder<::SimpleFoo>&,
        Args... args)
    {
        ++params.counter;
        return backend;
    }

    // describe struct ::SimpleBar

    template<class backend_type, class T=::SimpleBar, class U=::SimpleBar, class... Args>
    inline backend_type& parameter(
        backend_type& backend,
        ::SimpleBar& params, 
        const TypeHolder<::SimpleBar>&,
        Args... args)
    {
        ++params.counter;
        return backend;
    }

} // namespace im_param

int forward_from_backend(int argc, char* argv[])
{
    {
        Backend backend;
        SimpleFoo simple;
        ASSERT(simple.counter == 0);
        im_param::parameter(backend, "simple", simple, im_param::TypeHolder<SimpleFoo>());
        ASSERT(simple.counter == 1);
    }

    {
        Backend backend;
        SimpleBar simple;
        ASSERT(simple.counter == 0);
        im_param::parameter(backend, "simple", simple, im_param::TypeHolder<SimpleBar>());
        ASSERT(simple.counter == 1);
    }

    return 0;
}
