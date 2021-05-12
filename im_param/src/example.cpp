#include <iostream>
#include "im_param/im_param.h"
#include "im_param/backends/json_backend.h"

struct Bar
{
    struct Parameters
    {
        float val;
    };
    Parameters params;
};

template<class T>
struct Foobar
{
    struct Parameters
    {
        Bar::Parameters bar;
        float           a;
        int             b;
        bool            c;
        T               d;
    };
    Parameters params;
};

namespace im_param {

    template<class backend_type>
    backend_type& parameter(
        backend_type& backend,
        typename Bar::Parameters& params, 
        const TypeHolder<Bar>&)
    {
        parameter(backend, "val", params.val);
        return backend;
    }


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
        return backend;
    }

}

int main(int argc, char **argv)
{
    Foobar<double> foobar;
    foobar.params.bar.val = 1;
    foobar.params.a = 2;
    foobar.params.b = 3;
    foobar.params.c = 4;
    foobar.params.d = 5;
    im_param::JsonSerializerBackend jsonSerializer;
    im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
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
    //         "d": 5.0
    //     }
    // }    
    
    im_param::JsonDeserializerBackend jsonDeserializer(jsonSerializer.json_string());
    im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;

    jsonDeserializer.parse("{\"foobar\": {\"a\": 6, \"bar\": {}}}");
    im_param::parameter(jsonDeserializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
    std::cout << "jsonDeserializer.changed " << jsonDeserializer.changed << std::endl;
    
    jsonSerializer.clear();
    im_param::parameter(jsonSerializer, "foobar", foobar.params, im_param::TypeHolder<decltype(foobar)>());
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
    //         "d": 5.0
    //     }
    // }    
}
