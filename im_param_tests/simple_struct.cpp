

#include <im_param/im_param.h>
#include <im_param/backends/json_backend.h>

#include "tests_common.h"

struct SimpleStruct
{
    float a = 0;
    int b = 0;
};

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

} // namespace im_param

int simple_struct(int argc, char* argv[])
{


    SimpleStruct foo;
    SimpleStruct bar;

    foo.a = 1;
    foo.b = 2;
    bar.a = 3;
    bar.b = 4;

    {
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::JsonDeserializerBackend jsonDeserializer;
        im_param::parameter(jsonSerializer, "foo", foo, im_param::TypeHolder<SimpleStruct>());
        std::string foo_json = jsonSerializer.json_string(4);
        std::string foo_json_truth = std::string(
            "{"                   "\n"
            "    \"foo\": {"      "\n"
            "        \"a\": 1.0," "\n"
            "        \"b\": 2"    "\n"
            "    }"               "\n"
            "}"
        );
        std::cout << "foo_json "       << foo_json << "\n";
        std::cout << "foo_json_truth " << foo_json_truth << "\n";
        ASSERT(foo_json == foo_json_truth);

        SimpleStruct deserialized;
        jsonDeserializer.parse(foo_json);
        im_param::parameter(jsonDeserializer, "foo", deserialized, im_param::TypeHolder<SimpleStruct>());

        ASSERT(deserialized.a == foo.a);
        ASSERT(deserialized.b == foo.b);
    }

    {
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::JsonDeserializerBackend jsonDeserializer;
        im_param::parameter(jsonSerializer, "bar", bar, im_param::TypeHolder<SimpleStruct>());
        std::string bar_json = jsonSerializer.json_string(4);
        std::string bar_json_truth = std::string(
            "{"                   "\n"
            "    \"bar\": {"      "\n"
            "        \"a\": 3.0," "\n"
            "        \"b\": 4"    "\n"
            "    }"               "\n"
            "}"
        );
        std::cout << "bar_json "       << bar_json << "\n";
        std::cout << "bar_json_truth " << bar_json_truth << "\n";
        ASSERT(bar_json == bar_json_truth);

        SimpleStruct deserialized;
        jsonDeserializer.parse(bar_json);
        im_param::parameter(jsonDeserializer, "bar", deserialized, im_param::TypeHolder<SimpleStruct>());

        ASSERT(deserialized.a == bar.a);
        ASSERT(deserialized.b == bar.b);
    }

    {
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::JsonDeserializerBackend jsonDeserializer;
        im_param::parameter(jsonSerializer, "foo", foo, im_param::TypeHolder<SimpleStruct>());
        im_param::parameter(jsonSerializer, "bar", bar, im_param::TypeHolder<SimpleStruct>());
        std::string foo_bar_json = jsonSerializer.json_string(4);
        std::string foo_bar_json_truth = std::string(
            "{"                   "\n"
            "    \"bar\": {"      "\n"
            "        \"a\": 3.0," "\n"
            "        \"b\": 4"    "\n"
            "    },"              "\n"
            "    \"foo\": {"      "\n"
            "        \"a\": 1.0," "\n"
            "        \"b\": 2"    "\n"
            "    }"               "\n"
            "}"
        );
        std::cout << "foo_bar_json "       << foo_bar_json << "\n";
        std::cout << "foo_bar_json_truth " << foo_bar_json_truth << "\n";
        ASSERT(foo_bar_json == foo_bar_json_truth);

        SimpleStruct deserialized_foo;
        SimpleStruct deserialized_bar;
        jsonDeserializer.parse(foo_bar_json);
        im_param::parameter(jsonDeserializer, "foo", deserialized_foo, im_param::TypeHolder<SimpleStruct>());
        im_param::parameter(jsonDeserializer, "bar", deserialized_bar, im_param::TypeHolder<SimpleStruct>());

        ASSERT(deserialized_foo.a == foo.a);
        ASSERT(deserialized_foo.b == foo.b);
        ASSERT(deserialized_bar.a == bar.a);
        ASSERT(deserialized_bar.b == bar.b);
    }

    {
        im_param::JsonSerializerBackend jsonSerializer;
        im_param::JsonDeserializerBackend jsonDeserializer;
        im_param::parameter(jsonSerializer, "foo", foo, im_param::TypeHolder<SimpleStruct>());
        jsonSerializer.clear();
        im_param::parameter(jsonSerializer, "bar", bar, im_param::TypeHolder<SimpleStruct>());
        std::string foo_clear_bar_json = jsonSerializer.json_string(4);
        std::string foo_clear_bar_json_truth = std::string(
            "{"                   "\n"
            "    \"bar\": {"      "\n"
            "        \"a\": 3.0," "\n"
            "        \"b\": 4"    "\n"
            "    }"               "\n"
            "}"
        );
        std::cout << "foo_clear_bar_json "       << foo_clear_bar_json << "\n";
        std::cout << "foo_clear_bar_json_truth " << foo_clear_bar_json_truth << "\n";
        ASSERT(foo_clear_bar_json == foo_clear_bar_json_truth);

        SimpleStruct deserialized;
        jsonDeserializer.parse(foo_clear_bar_json);
        im_param::parameter(jsonDeserializer, "bar", deserialized, im_param::TypeHolder<SimpleStruct>());

        ASSERT(deserialized.a == bar.a);
        ASSERT(deserialized.b == bar.b);
        
    }

    return 0;
}
