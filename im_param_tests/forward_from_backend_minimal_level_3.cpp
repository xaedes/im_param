#include <iostream>
#include <string>

#include "tests_common.h"

namespace ns {
    
    template<class BE, class UT>
    struct UserLand
    {
        static void spezialization(BE& be, UT& ut) {}
    };
    
    template<class BE, class UT>
    void userland(BE& be, UT& ut)
    {
        UserLand<BE,UT>::spezialization(be, ut);
    }
    
    template<class BE, class UT>
    void kickoff(BE& be, const char* label, UT& ut)
    {
        be.work_backend(label, ut);
    }
    
} // namespace ns

struct Backend
{
    int called_with_int = 0;
    int called_with_ut = 0;
    void work_backend(const char* label, int& ut)
    {
        std::cout << "Backend::work_backend('" << label << "', " << ut << ")" << "\n";
        ++called_with_int;
        ns::userland(*this, ut);
    }
    
    template<class UT>
    void work_backend(const char* label, UT& ut)
    {
        std::cout << "Backend::work_backend('" << label << "', ut)" << "\n";
        ++called_with_ut;
        ns::userland(*this, ut);
    }
};

struct Foo
{
    int val=0;
};

namespace ns {
    
    /*
    #define BEGIN(be,UT,ut) template<class BE> struct UserLand<BE, UT> { static void spezialization(BE& be, UT& ut)
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
        static void spezialization(BE& be, int& ut)
        {
            ut++;
        }
    };
    
    template<class BE>
    struct UserLand<BE, Foo>
    {
        static void spezialization(BE& be, Foo& ut)
        {
            ns::kickoff(be, "val", ut.val);
        }
    };
    //*/
}

int forward_from_backend_minimal_level_3(int argc, char* argv[]) {

    // a call chain can look like this:
    // 
    // kickoff 
    //  -> backend 
    //   -> userland 
    //    -> spezialization 
    //     -> kickoff 
    //      -> backend 
    //       -> userland 
    //        -> spezialization 
    //         -> ...
    //     -> kickoff 
    //      -> backend 
    //       -> userland 
    //        -> spezialization 
    //         -> ...
    //
    // context                         | callable
    // --------------------------------+---------
    // usercode                        | ns::kickoff, ns::userland
    // ns::kickoff                     | BE::work_backend
    // BE::work_backend                | ns::userland, ns::kickoff
    // ns::userland                    | UserLand<BE,UT>::spezialization
    // UserLand<BE,UT>::spezialization | ns::kickoff
    // 
    // the difference between ns::kickoff and ns::userland is that ns::kickoff
    // takes additional arguments that can be consumed by the backend

    Backend backend;
    Foo foo{0};
    ASSERT(foo.val == 0);
    
    ns::kickoff(backend, "foo", foo);
    ASSERT(foo.val == 1);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 1);
    
    int x=1;
    
    ns::kickoff(backend, "x", x);
    ASSERT(x == 2);
    ASSERT(backend.called_with_int == 2);
    
    ns::kickoff(backend, "x", x);
    ASSERT(x == 3);
    ASSERT(backend.called_with_int == 3);

    return 0;
}
    