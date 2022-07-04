

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

// testing generic and nested types

struct SimpleStruct
{
    float a = 0;
    int b = 0;
};

struct OuterClass
{
    struct Parameters
    {
        SimpleStruct simple;
        float c = 0;
        int d = 0;
    };
};

template<class T>
struct GenericOuterFoo
{
    struct Parameters
    {
        SimpleStruct simple;
        float a = 0;
        int b = 0;
        T foo = 0;
    };

};

template<class U>
struct GenericOuterBar
{
    struct Parameters
    {
        typename GenericOuterFoo<U>::Parameters foo;
        SimpleStruct bar_simple;
        float a = 0;
        int b = 0;
        U bar = 0;
    };

};

// template<class V>
// struct GenericOuterFoobar
// {
//     template<class W>
//     struct GenericParameters
//     {
//         GenericOuterFoo<V>::Parameters foo;
//         GenericOuterBar<W>::Parameters bar;
//         SimpleStruct bar_simple;
//         float a = 0;
//         int b = 0;
//         V foo_baz = 0;
//         W bar_baz = 0;
//     };

// };

namespace im_param {

    // describe struct ::SimpleStruct
    template<class backend_type>
    backend_type& parameter(
        backend_type& backend,
        ::SimpleStruct& params, 
        const TypeHolder<::SimpleStruct>&)
    {
        parameter(backend, "a", params.a);
        parameter(backend, "b", params.b);
        return backend;
    }

    // describe struct ::OuterClass::Parameters
    template<class backend_type>
    backend_type& parameter(
        backend_type& backend,
        ::OuterClass::Parameters& params, 
        const TypeHolder<::OuterClass::Parameters>&)
    {
        parameter(backend, "simple", params.simple, TypeHolder<::SimpleStruct>());
        parameter(backend, "c", params.c);
        parameter(backend, "d", params.d);
        return backend;
    }

    // describe struct ::GenericOuterFoo<T>::Parameters 
    // 
    // note the use of `typename` for params type. 
    // 
    // note the type of TypeHolder: it contains the
    // templated type ::GenericOuterFoo<T> instead of
    // ::GenericOuterFoo<T>::Parameters. this is necessary 
    // for the compiler to catch type T. this is actually
    // the reason for the required TypeHolder argument.
    template<class backend_type, class T>
    backend_type& parameter(
        backend_type& backend,
        typename ::GenericOuterFoo<T>::Parameters& params, 
        const TypeHolder<::GenericOuterFoo<T>>&)
    {
        parameter(backend, "simple", params.simple, TypeHolder<::SimpleStruct>());
        parameter(backend, "a", params.a);
        parameter(backend, "b", params.b);
        parameter(backend, "foo", params.foo);
        return backend;
    }

    // describe struct ::GenericOuterBar<T>::Parameters 
    template<class backend_type, class T>
    backend_type& parameter(
        backend_type& backend,
        typename ::GenericOuterBar<T>::Parameters& params, 
        const TypeHolder<::GenericOuterBar<T>>&)
    {
        parameter(backend, "foo", params.foo, TypeHolder<::GenericOuterFoo<T>>());
        parameter(backend, "bar_simple", params.bar_simple, TypeHolder<::SimpleStruct>());
        parameter(backend, "a", params.a);
        parameter(backend, "b", params.b);
        parameter(backend, "bar", params.bar);
        return backend;
    }

    // // describe struct ::GenericOuterFoobar<U>::GenericParameters<V>
    // template<class backend_type, class U, class V>
    // backend_type& parameter(
    //     backend_type& backend,
    //     typename ::GenericOuterBar<U>::Parameters<V>& params, 
    //     const TypeHolder<::GenericOuterBar<T>>&)
    // {
    //     parameter(backend, "foo", params.foo, TypeHolder<::GenericOuterFoo<T>>());
    //     parameter(backend, "bar_simple", params.bar_simple, TypeHolder<::SimpleStruct>());
    //     parameter(backend, "a", params.a);
    //     parameter(backend, "b", params.b);
    //     parameter(backend, "bar", params.bar);
    //     return backend;
    // }


} // namespace im_param

int generic_and_nested_types(int argc, char* argv[])
{

    im_param::JsonDeserializerBackend jsonDeserializer;

    SimpleStruct simple_struct;
    OuterClass::Parameters outer_class;
    GenericOuterFoo<double>::Parameters generic_outer_foo_double;
    GenericOuterBar<double>::Parameters generic_outer_bar_double;

    simple_struct.a                              = 1;
    simple_struct.b                              = 2;
    outer_class.simple.a                         = 3;
    outer_class.simple.b                         = 4;
    outer_class.c                                = 5;
    outer_class.d                                = 6;
    generic_outer_foo_double.simple.a            = 7;
    generic_outer_foo_double.simple.b            = 8;
    generic_outer_foo_double.a                   = 9;
    generic_outer_foo_double.b                   = 10;
    generic_outer_foo_double.foo                 = 11;

    generic_outer_bar_double.foo.simple.a        = 12;
    generic_outer_bar_double.foo.simple.b        = 13;
    generic_outer_bar_double.foo.a               = 14;
    generic_outer_bar_double.foo.b               = 15;
    generic_outer_bar_double.foo.foo             = 16;
    generic_outer_bar_double.bar_simple.a        = 17;
    generic_outer_bar_double.bar_simple.b        = 18;
    generic_outer_bar_double.a                   = 19;
    generic_outer_bar_double.b                   = 20;
    generic_outer_bar_double.bar                 = 21;

    {
        // test SimpleStruct simple_struct
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::parameter(jsonSerializer, "simple_struct", simple_struct, im_param::TypeHolder<SimpleStruct>());

        std::string simple_struct_json = jsonSerializer.json_string(4);
        std::string simple_struct_json_truth = std::string(
            "{"                        "\n"
            "    \"simple_struct\": {" "\n"
            "        \"a\": 1.0,"      "\n"
            "        \"b\": 2"         "\n"
            "    }"                    "\n"
            "}"
        );
        std::cout << "simple_struct_json "       << simple_struct_json << "\n";
        std::cout << "simple_struct_json_truth " << simple_struct_json_truth << "\n";
        ASSERT(simple_struct_json == simple_struct_json_truth);
    }
    {
        // test OuterClass::Parameters outer_class
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::parameter(jsonSerializer, "outer_class", outer_class, im_param::TypeHolder<OuterClass::Parameters>());

        std::string outer_class_json = jsonSerializer.json_string(4);
        std::string outer_class_json_truth = std::string(
            "{"                        "\n"
            "    \"outer_class\": {"   "\n"
            "        \"c\": 5.0,"      "\n"
            "        \"d\": 6,"        "\n"
            "        \"simple\": {"    "\n"
            "            \"a\": 3.0,"  "\n"
            "            \"b\": 4"     "\n"
            "        }"                "\n"
            "    }"                    "\n"
            "}"
        );
        std::cout << "outer_class_json "       << outer_class_json << "\n";
        std::cout << "outer_class_json_truth " << outer_class_json_truth << "\n";
        ASSERT(outer_class_json == outer_class_json_truth);
    }
    {
        // test ::GenericOuterFoo<double>::Parameters  generic_outer_foo_double
        im_param::JsonSerializerBackend jsonSerializer;
        // note the type of TypeHolder
        im_param::parameter(jsonSerializer, "generic_outer_foo_double", generic_outer_foo_double, im_param::TypeHolder<GenericOuterFoo<double>>());

        std::string generic_outer_foo_double_json = jsonSerializer.json_string(4);
        std::string generic_outer_foo_double_json_truth = std::string(
            "{"                                     "\n"
            "    \"generic_outer_foo_double\": {"   "\n"
            "        \"a\": 9.0,"                   "\n"
            "        \"b\": 10,"                    "\n"
            "        \"foo\": 11.0,"                "\n"
            "        \"simple\": {"                 "\n"
            "            \"a\": 7.0,"               "\n"
            "            \"b\": 8"                  "\n"
            "        }"                             "\n"
            "    }"                                 "\n"
            "}"
        );
        std::cout << "generic_outer_foo_double_json "       << generic_outer_foo_double_json << "\n";
        std::cout << "generic_outer_foo_double_json_truth " << generic_outer_foo_double_json_truth << "\n";
        ASSERT(generic_outer_foo_double_json == generic_outer_foo_double_json_truth);
    }

    {
        // test ::GenericOuterBar<double>::Parameters generic_outer_bar_double
        im_param::JsonSerializerBackend jsonSerializer;
        // note the type of TypeHolder
        im_param::parameter(jsonSerializer, "generic_outer_bar_double", generic_outer_bar_double, im_param::TypeHolder<GenericOuterBar<double>>());

        std::string generic_outer_bar_double_json = jsonSerializer.json_string(4);
        std::string generic_outer_bar_double_json_truth = std::string(
            "{"                                    "\n"
            "    \"generic_outer_bar_double\": {"  "\n"
            "        \"a\": 19.0,"                 "\n"
            "        \"b\": 20,"                   "\n"
            "        \"bar\": 21.0,"               "\n"
            "        \"bar_simple\": {"            "\n"
            "            \"a\": 17.0,"             "\n"
            "            \"b\": 18"                "\n"
            "        },"                           "\n"
            "        \"foo\": {"                   "\n"
            "            \"a\": 14.0,"             "\n"
            "            \"b\": 15,"               "\n"
            "            \"foo\": 16.0,"           "\n"
            "            \"simple\": {"            "\n"
            "                \"a\": 12.0,"         "\n"
            "                \"b\": 13"            "\n"
            "            }"                        "\n"
            "        }"                            "\n"
            "    }"                                "\n"
            "}"                                    
        );
        std::cout << "generic_outer_bar_double_json "       << generic_outer_bar_double_json << "\n";
        std::cout << "generic_outer_bar_double_json_truth " << generic_outer_bar_double_json_truth << "\n";
        ASSERT(generic_outer_bar_double_json == generic_outer_bar_double_json_truth);
    }

    return 0;
}
