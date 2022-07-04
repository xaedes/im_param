

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

// a vector with multiple base values 
template<class T, int N>
struct Vec
{
    T values[N];
    Vec() : Vec(0) {}
    Vec(T val)
    {
        for (int i = 0; i < N; ++i)
        {
            values[i] = val;
        }
    }
};


struct MyStruct
{
    Vec<float, 3> vec3f;
    Vec<int, 2> vec2i;
};

namespace im_param {

    // describe multi base valued types
    template<
        class backend_type,
        class value_type,
        int N,
        std::enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        ::Vec<value_type, N>& value, 
        ::Vec<value_type, N> min = 0, 
        ::Vec<value_type, N> max = 1
    )
    {
        parameter(backend, name, value.values, N, min.values, max.values);
        return backend;
    }

    // use above description of Vec in description of MyStruct
    template<class backend_type, class... Args>
    backend_type& parameter(
        backend_type& backend,
        ::MyStruct& params, 
        const TypeHolder<::MyStruct>&,
        Args... args)
    {
        parameter(backend, "vec3f", params.vec3f);
        parameter(backend, "vec2i", params.vec2i);
        return backend;
    }
} // namespace im_param

int multi_base_value(int argc, char* argv[])
{

    im_param::JsonSerializerBackend jsonSerializer;
    im_param::JsonDeserializerBackend jsonDeserializer;

    MyStruct params;

    params.vec3f.values[0] = 1;
    params.vec3f.values[1] = 2;
    params.vec3f.values[2] = 3;
    params.vec2i.values[0] = 4;
    params.vec2i.values[1] = 5;
    
    im_param::parameter(jsonSerializer, "params", params, im_param::TypeHolder<MyStruct>());

    std::string json = jsonSerializer.json_string(4);
    // note the lexicographic order of vec2i and vec3f, which is not the same
    // order as described in im_param::parameter declaration.
    std::string json_truth = std::string(
        "{"                     "\n"
        "    \"params\": {"     "\n"
        "        \"vec2i\": ["  "\n"
        "            4,"        "\n"
        "            5"         "\n"
        "        ],"            "\n"
        "        \"vec3f\": ["  "\n"
        "            1.0,"      "\n"
        "            2.0,"      "\n"
        "            3.0"       "\n"
        "        ]"             "\n"
        "    }"                 "\n"
        "}"
    );
    std::cout << "json " << json << "\n";
    std::cout << "json_truth " << json_truth << "\n";
    ASSERT(json == json_truth);

    return 0;
}
