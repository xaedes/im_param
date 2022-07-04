

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

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

struct SimpleStruct
{
    int counter = 0;
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
        ++params.counter;
        return backend;
    }

} // namespace im_param

int forward_from_backend(int argc, char* argv[])
{
    {
        Backend backend;
        SimpleStruct simple_struct;
        ASSERT(simple_struct.counter == 0);
        im_param::parameter(backend, "simple_struct", simple_struct, im_param::TypeHolder<SimpleStruct>());
        ASSERT(simple_struct.counter == 1);
    }

    return 0;
}
