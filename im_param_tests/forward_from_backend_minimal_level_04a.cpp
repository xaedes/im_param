#include <iostream>
#include <string>

#include "tests_common.h"

// rename every function to "parameter"
// works, simple, but hard to follow, hence hard to debug

namespace lvl04a {

namespace im_param {
    
    template<class BE, class UT>
    struct Custom
    {
        static void parameter(BE& be, UT& ut) {}
    };
    
    template<class BE, class UT>
    void parameter(BE& be, UT& ut)
    {
        Custom<BE,UT>::parameter(be, ut);
    }
    
    template<class BE, class UT>
    void parameter(BE& be, const char* label, UT& ut)
    {
        be.parameter(label, ut);
    }
    
} // namespace im_param

struct Backend
{
    int called_with_int = 0;
    int called_with_ut = 0;
    void parameter(const char* label, int& ut)
    {
        std::cout << "Backend::parameter('" << label << "', " << ut << ")" << "\n";
        ++called_with_int;
        im_param::parameter(*this, ut);
    }
    
    template<class UT>
    void parameter(const char* label, UT& ut)
    {
        std::cout << "Backend::parameter('" << label << "', ut)" << "\n";
        ++called_with_ut;
        im_param::parameter(*this, ut);
    }
};

struct Foo
{
    int val=0;
};

namespace im_param {

    template<class BE>
    struct Custom<BE, int>
    {
        static void parameter(BE& be, int& ut)
        {
            ut++;
        }
    };
    
    template<class BE>
    struct Custom<BE, Foo>
    {
        static void parameter(BE& be, Foo& ut)
        {
            im_param::parameter(be, "val", ut.val);
        }
    };

} // namespace im_param

} // namespace lvl04a

int forward_from_backend_minimal_level_04a(int argc, char* argv[]) {

    // a call chain can look like this:
    // 
    // parameter 
    //  -> parameter 
    //   -> parameter 
    //    -> parameter 
    //     -> parameter 
    //      -> parameter 
    //       -> parameter 
    //        -> parameter 
    //         -> ...
    //     -> parameter 
    //      -> parameter 
    //       -> parameter 
    //        -> parameter 
    //         -> ...
    //
    // context                   | callable
    // --------------------------+---------
    // usercode                  | im_param::parameter
    // im_param::parameter       | BE::parameter
    // BE::parameter             | im_param::parameter
    // im_param::parameter       | Custom<BE,UT>::parameter
    // Custom<BE,UT>::parameter  | im_param::parameter

    lvl04a::Backend backend{};
    lvl04a::Foo foo{};
    foo.val = 0;
    
    lvl04a::im_param::parameter(backend, "foo", foo);
    ASSERT(foo.val == 1);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 1);
    
    int x=1;
    
    lvl04a::im_param::parameter(backend, "x", x);
    ASSERT(x == 2);
    ASSERT(backend.called_with_int == 2);
    
    lvl04a::im_param::parameter(backend, "x", x);
    ASSERT(x == 3);
    ASSERT(backend.called_with_int == 3);

    return 0;
}
    