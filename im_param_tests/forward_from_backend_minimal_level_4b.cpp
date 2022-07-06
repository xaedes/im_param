#include <iostream>
#include <string>

#include "tests_common.h"

// provide "parameter" synonyms using wrapper functions and types
// both naming variants can be used in userland code

namespace lvl4b {
namespace im_param {
    
    template<class BE, class UT>
    struct Custom
    {
        static void parameter(BE& be, UT& ut) 
        { /* empty */ }
    };
    
    // default UserLand implementation will forward to Custom::parameter synonyms.
    // UserLand<BE,T> can be specialized for T by users or they specialize Custom<BE,T>.
    template<class BE, class UT>
    struct UserLand
    {
        static void specialization(BE& be, UT& ut) 
        {
            Custom<BE, UT>::parameter(be, ut);
        }
    };

    // go into userland code
    template<class BE, class UT>
    void userland(BE& be, UT& ut)
    {
        UserLand<BE,UT>::specialization(be, ut);
    }
    
    // kick off the call chain by going into the backend BE
    template<class BE, class UT>
    void kickoff(BE& be, const char* label, UT& ut)
    {
        be.work_backend(label, ut);
    }

} // namespace im_param

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{
    int called_with_int = 0;
    int called_with_ut = 0;

    void work_backend(const char* label, int& ut)
    {
        std::cout << "Backend::work_backend('" << label << "', " << ut << ")" << "\n";
        ++called_with_int;
        im_param::userland(*this, ut);
    }
    
    template<class UT>
    void work_backend(const char* label, UT& ut)
    {
        std::cout << "Backend::work_backend('" << label << "', ut)" << "\n";
        ++called_with_ut;
        im_param::userland(*this, ut);
    }
};

//-----------------------------------------------------------------------------

namespace im_param {

    // `parameter` synonyms

    template<class BE, class UT>
    void parameter(BE& be, UT& ut)
    {
        userland(be, ut);
    }
    
    template<class BE, class UT>
    void parameter(BE& be, const char* label, UT& ut)
    {
        kickoff(be, label, ut);
    }
    
} // namespace im_param

//-----------------------------------------------------------------------------

struct Foo
{
    int val=0;
};

struct Bar
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
    
    // using Custom<BE, ?> users can define via `parameter` synonyms
    template<class BE>
    struct Custom<BE, Foo>
    {
        static void parameter(BE& be, Foo& ut)
        {
            // we may explicitly use the named kickoff function
            im_param::kickoff(be, "val", ut.val);
            // or we use the `parameter` synonym
            // im_param::parameter(be, "val", ut.val);
        }
    };

    // using UserLand<BE, ?> users can define via explicit functions (e.g. 'specialization')
    template<class BE>
    struct UserLand<BE, Bar>
    {
        static void specialization(BE& be, Bar& ut)
        {
            im_param::parameter(be, "val", ut.val);
        }
    };

} // namespace im_param
} // namespace lvl4b

int forward_from_backend_minimal_level_4b(int argc, char* argv[]) {

    lvl4b::Backend backend{};
    lvl4b::Foo foo{};
    foo.val = 0;
    
    lvl4b::im_param::parameter(backend, "foo", foo);
    ASSERT(foo.val == 1);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 1);
    
    int x=1;
    
    lvl4b::im_param::parameter(backend, "x", x);
    ASSERT(x == 2);
    ASSERT(backend.called_with_int == 2);
    
    lvl4b::im_param::parameter(backend, "x", x);
    ASSERT(x == 3);
    ASSERT(backend.called_with_int == 3);

    return 0;
}
    