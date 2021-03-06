#include <iostream>
#include <string>

#include "tests_common.h"


namespace lvl03 {

namespace im_param {
    
    template<class BE, class UT>
    struct UserLand
    {
        static void specialization(BE& be, UT& ut) {}
    };
    
    template<class BE, class UT>
    void userland(BE& be, UT& ut)
    {
        UserLand<BE,UT>::specialization(be, ut);
    }
    
    template<class BE, class UT>
    void kickoff(BE& be, const char* label, UT& ut)
    {
        be.work_backend(label, ut);
    }
    
} // namespace im_param

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

struct Foo
{
    int val=0;
};

namespace im_param {
    
    /*
    #define BEGIN(be,UT,ut) template<class BE> struct UserLand<BE, UT> { static void specialization(BE& be, UT& ut)
    #define END() };
    
    BEGIN(be, int, val)
    {
        val++;
    }
    END()
    
    BEGIN(be, Foo, val)
    {
        nsq::kickoff(be, "val", val.val);
    }
    END()
    
    
    /*/

    template<class BE>
    struct UserLand<BE, int>
    {
        static void specialization(BE& be, int& ut)
        {
            ut++;
        }
    };
    
    template<class BE>
    struct UserLand<BE, Foo>
    {
        static void specialization(BE& be, Foo& ut)
        {
            im_param::kickoff(be, "val", ut.val);
        }
    };
    //*/
} // namespace im_param

} // namespace lvl03

int im_param_foundations_level_03(int argc, char* argv[]) {
    // a call chain can look like this:
    // 
    // kickoff 
    //  -> backend 
    //   -> userland 
    //    -> specialization 
    //     -> kickoff 
    //      -> backend 
    //       -> userland 
    //        -> specialization 
    //         -> ...
    //     -> kickoff 
    //      -> backend 
    //       -> userland 
    //        -> specialization 
    //         -> ...
    //
    // context                         | callable
    // --------------------------------+---------
    // usercode                        | im_param::kickoff, im_param::userland
    // im_param::kickoff               | BE::work_backend
    // BE::work_backend                | im_param::userland, im_param::kickoff
    // im_param::userland              | UserLand<BE,UT>::specialization
    // UserLand<BE,UT>::specialization | im_param::kickoff
    // 
    // the difference between im_param::kickoff and im_param::userland is that im_param::kickoff
    // takes additional arguments that can be consumed by the backend

    lvl03::Backend backend{};
    lvl03::Foo foo{};
    foo.val = 0;
    
    lvl03::im_param::kickoff(backend, "foo", foo);
    ASSERT(foo.val == 1);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 1);
    
    int x=1;
    
    lvl03::im_param::kickoff(backend, "x", x);
    ASSERT(x == 2);
    ASSERT(backend.called_with_int == 2);
    
    lvl03::im_param::kickoff(backend, "x", x);
    ASSERT(x == 3);
    ASSERT(backend.called_with_int == 3);

    return 0;
}
    