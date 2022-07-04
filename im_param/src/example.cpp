#include <iostream>
#include <vector>
#include "im_param/im_param.h"
#include "im_param/std_collections.h"
#include "im_param/backends/json_backend.h"
#include "im_param/backends/glsl_struct_generator_backend.h"

///////////////////////////////////
// example data structures

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

// a simple struct
struct Bar
{
    struct Parameters
    {
        float val = 0;
    };
    Parameters params;
};

// a more complex struct
template<class T>
struct Foobar
{
    struct Parameters
    {
        Bar::Parameters bar;
        float           a = 0;
        int             b = 0;
        bool            c = 0;
        T               d = 0;
        Vec<T, 3>       e;
        T               untouched;
    };
    Parameters params;
};

namespace im_param {

    // describe multi base valued types
    template<
        typename backend_type,
        typename value_type,
        int N,
        enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        //typename A = value_type, typename B = value_type
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        Vec<value_type, N>& value, 
        Vec<value_type, N> min = 0, 
        Vec<value_type, N> max = 1
    )
    {
        parameter(backend, name, value.values, N, min.values, max.values);
        return backend;
    }

    // describe list of multi base valued types
    template<
        typename backend_type,
        typename collection_type,
        typename value_type,
        typename value_iterator_type,
        typename inserter_iterator_type,
        int N,
        enable_if_t<Backend::is_base_value<value_type>::value, bool> = true
        // enable_if_t<std::is_same<value_iterator_type::value_type,>::value, bool> = true
        //typename A = value_type, typename B = value_type
    >
    backend_type& parameter(
        backend_type& backend, 
        const std::string& name, 
        Collection<collection_type,Vec<value_type, N>,value_iterator_type,inserter_iterator_type>& collection,
        Vec<value_type, N> min = 0, 
        Vec<value_type, N> max = 1
    )
    {
        parameter(
            backend, 
            name, 
            collection, 
            [](Vec<value_type, N>& item) {return item.values; },
            N, 
            min.values, 
            max.values
        );
        return backend;
    }

    // template<
    //     typename backend_type,
    //     typename value_type,
    //     int N,
    //     //typename A = value_type, typename B = value_type
    // >
    // backend_type& parameter(backend_type& backend, Vec<value_type, N>& value, Vec<value_type, N> min = 0, Vec<value_type, N> max = 1)
    // {
    //     parameter(backend, value.values, N, min.values, max.values);
    //     return backend;
    // }

    // describe Bar
    template<class backend_type>
    backend_type& parameter(
        backend_type& backend,
        typename Bar::Parameters& params, 
        const TypeHolder<Bar>&)
    {
        parameter(backend, "val", params.val);
        return backend;
    }

    // describe Foobar
    template<class backend_type, class T>
    backend_type& parameter(
        backend_type& backend,
        typename Foobar<T>::Parameters& params, 
        const TypeHolder<Foobar<T>>&)
    {
        parameter(backend, "bar", params.bar, TypeHolder<Bar>());
        parameter(backend, "a", params.a);
        parameter(backend, "b", params.b);
        parameter(backend, "c", params.c);
        parameter(backend, "d", params.d);
        parameter(backend, "e", params.e);
        parameter(backend, "untouched", params.untouched);
        return backend;
    }

}

int main(int argc, char **argv)
{
    Foobar<double> foobar;
    foobar.params.bar.val = 1;
    foobar.params.a = 2;
    foobar.params.b = 3;
    foobar.params.c = false;
    foobar.params.d = 5;
    foobar.params.e.values[0] = 6;
    foobar.params.e.values[1] = 7;
    foobar.params.e.values[2] = 8;

    im_param::JsonSerializerBackend jsonSerializer;
    im_param::JsonDeserializerBackend jsonDeserializer;

    #pragma region im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << R"(im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    // Output:
    // {
    //     "foobar": {
    //         "a": 2.0,
    //         "b": 3,
    //         "bar": {
    //             "val": 1.0
    //         },
    //         "c": true,
    //         "d": 5.0,
    //         "e": [
    //             6.0,
    //             7.0,
    //             8.0
    //         ]
    //     }
    // }
    #pragma endregion
    
    #pragma region jsonDeserializer.parse(jsonSerializer.json_string()));  im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    jsonDeserializer.clear();
    jsonDeserializer.parse(jsonSerializer.json_string());
    im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << R"(jsonDeserializer.parse(jsonSerializer.json_string()))" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"foobar": {"a": 6, "bar": {}}})"); im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    jsonDeserializer.clear();
    jsonDeserializer.parse(R"({"foobar": {"a": 6, "bar": {}}})");
    im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << R"(jsonDeserializer.parse(''{"foobar": {"a": 6, "bar": {}}}'');)" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion
    
    
    #pragma region im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << R"(im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    // Output:
    // jsonDeserializer.changed 0
    // jsonDeserializer.changed 1
    // {
    //     "foobar": {
    //         "a": 6.0,
    //         "b": 3,
    //         "bar": {
    //             "val": 1.0
    //         },
    //         "c": true,
    //         "d": 5.0,
    //         "e": [
    //             6.0,
    //             7.0,
    //             8.0
    //         ]
    //     }
    // }
    #pragma endregion

    
    std::vector<int> integers{1,2,3,4};
    std::vector<Vec<float,3>> vectors{
        Vec<float,3>(0.0f),
        Vec<float,3>(1.0f),
        Vec<float,3>(2.0f)
    };
    std::vector<Foobar<double>::Parameters> foobars{
        foobar.params,
        foobar.params
    };
    
    #pragma region im_param::parameter(jsonSerializer, "integers", integers);)
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "integers", integers);
    std::cout << R"(im_param::parameter(jsonSerializer, "integers", integers);)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"integers": [7,8]})"); im_param::parameter(jsonDeserializer, "integers", integers);
    jsonDeserializer.clear();
    jsonDeserializer.parse(R"({"integers": [7,8]})");
    im_param::parameter(jsonDeserializer, "integers", integers);
    std::cout << R"(jsonDeserializer.parse(''({"integers": [7,8]}'');)" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "integers", integers);)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "integers", integers);
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "integers", integers);
    std::cout << R"(im_param::parameter(jsonSerializer, "integers", integers);)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "vectors", vectors);
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "vectors", vectors);
    std::cout << R"(im_param::parameter(jsonSerializer, "vectors", vectors);)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(jsonSerializer.json_string()); im_param::parameter(jsonDeserializer, "vectors", vectors);
    jsonDeserializer.clear();
    jsonDeserializer.parse(jsonSerializer.json_string());
    im_param::parameter(jsonDeserializer, "vectors", vectors);
    std::cout << R"(jsonDeserializer.parse(jsonSerializer.json_string());)" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "vectors", vectors);)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "vectors", vectors);
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "vectors", vectors);
    std::cout << R"(im_param::parameter(jsonSerializer, "vectors", vectors);)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"vectors": [[3,4,5],[6,7,8]]})"); im_param::parameter(jsonDeserializer, "vectors", vectors);
    jsonDeserializer.clear();
    jsonDeserializer.parse(R"({"vectors": [[3,4,5],[6,7,8]]})");
    im_param::parameter(jsonDeserializer, "vectors", vectors);
    std::cout << R"(jsonDeserializer.parse(''({"vectors": [[3,4,5],[6,7,8]]})'');)" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "vectors", vectors);)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "vectors", vectors);
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "vectors", vectors);
    std::cout << R"(im_param::parameter(jsonSerializer, "vectors", vectors);)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(jsonSerializer.json_string()); im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonDeserializer.clear();
    jsonDeserializer.parse(jsonSerializer.json_string());
    im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(jsonDeserializer.parse(jsonSerializer.json_string());)" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"foobars": [{"a": 1, "bar": {}}]})"); im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonDeserializer.parse(R"({"foobars": [{"a": 1, "bar": {}}]})");
    im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(jsonDeserializer.parse(''{"foobars": [{"a": 1, "bar": {}}]}'')" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"foobars": []})"); im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonDeserializer.clear();
    jsonDeserializer.parse(R"({"foobars": []})");
    im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(jsonDeserializer.parse(''{"foobars": []}'')" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    #pragma region jsonDeserializer.parse(R"({"foobars": [{"a": 1, "bar": {}}, {"a": 2, "bar": {}}]})"); im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonDeserializer.parse(R"({"foobars": [{"a": 1, "bar": {}}, {"a": 2, "bar": {}}]})");
    im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(jsonDeserializer.parse(''{"foobars": [{"a": 1, "bar": {}}, {"a": 2, "bar": {}}]}'')" << std::endl;
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    #pragma endregion

    #pragma region im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>());
    std::cout << R"(im_param::parameter(jsonDeserializer, "foobars", foobars, im_param::TypeHolder<Foobar<double>>()));)" << std::endl;
    std::cout << jsonSerializer.json_string(4) << std::endl;
    #pragma endregion

    im_param::GlslStructGeneratorBackend glslStructGenerator;
    #pragma region im_param::parameter(glslStructGenerator, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    im_param::parameter(glslStructGenerator, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << "glslStructGenerator.glsl_string() \n" << glslStructGenerator.glsl_string() << std::endl;
    // Output:
    // glslStructGenerator.glsl_string()
    // foobar foobar;
    // struct foobar
    // {
    //   bar bar;
    //   float a;
    //   int b;
    //   bool c;
    //   double d;
    //   dvec3 e;
    // };
    // struct bar
    // {
    //   float val;
    // };
    #pragma endregion
    return 0;
}
