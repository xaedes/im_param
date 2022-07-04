

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

struct Backend
{
    int counter = 0;

    template<class... Args>
    Backend& parameter(Args... args)
    {
        ++counter;
        return *this;
    }

};

struct SimpleStruct
{
    float a = 0;
    int b = 0;
};

namespace im_param {

    // describe struct ::SimpleStruct

    template<class backend_type, class... Args>
    backend_type& parameter(
        backend_type& backend,
        ::SimpleStruct& params, 
        const TypeHolder<::SimpleStruct>&,
        Args... args)
    {
        parameter(backend, "a", params.a);
        parameter(backend, "b", params.b);
        return backend;
    }

} // namespace im_param

int forward_to_backend(int argc, char* argv[])
{
    {
        Backend backend;
        ASSERT(backend.counter == 0);
        int int_val = 0;
        im_param::parameter(backend, "int_val", int_val);
        ASSERT(backend.counter == 1);
    }

    {
        Backend backend;
        ASSERT(backend.counter == 0);
        SimpleStruct simple_struct;
        im_param::parameter(backend, "simple_struct", simple_struct, im_param::TypeHolder<SimpleStruct>());
        ASSERT(backend.counter == 1);
    }

    return 0;
}
