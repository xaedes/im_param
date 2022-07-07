#include <iostream>
#include <string>
#include <type_traits>

#include "tests_common.h"

// add typeholder support for custom types nested in templated types
// but now everything, even fundamental types, require typeholders, 
// which is cumbersome to use.

namespace lvl05 {
namespace im_param {
    
    template<class BE, class UT, class TH>
    struct Custom
    {
        static void parameter(BE& be, UT& ut, const TH& th) 
        { /* empty */ }
    };
    
    // default UserLand implementation will forward to Custom::parameter synonyms.
    // UserLand<BE,T> can be specialized for T by users or they specialize Custom<BE,T>.
    template<class BE, class UT, class TH>
    struct UserLand
    {
        static void specialization(BE& be, UT& ut, const TH& th) 
        {
            Custom<BE,UT,TH>::parameter(be, ut, th);
        }
    };

    // go into userland code
    template<class BE, class UT, class TH>
    void userland(BE& be, UT& ut, const TH& th)
    {
        UserLand<BE,UT,TH>::specialization(be, ut, th);
    }
    
    // kick off the call chain by going into the backend BE
    template<class BE, class UT, class TH>
    void kickoff(BE& be, const char* label, UT& ut, const TH& th)
    {
        be.work_backend(label, ut, th);
    }

} // namespace im_param

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{
    int called_with_int = 0;
    int called_with_ut = 0;

    template<class TH>
    void work_backend(const char* label, int& ut, const TH& th)
    {
        std::cout << "Backend::work_backend('" << label << "', " << ut << ")" << "\n";
        ++called_with_int;
        im_param::userland(*this, ut, th);
    }
    
    template<class UT, class TH>
    void work_backend(const char* label, UT& ut, const TH& th)
    {
        std::cout << "Backend::work_backend('" << label << "', ut)" << "\n";
        ++called_with_ut;
        im_param::userland(*this, ut, th);
    }
};

//-----------------------------------------------------------------------------

namespace im_param {

    // `parameter` synonyms

    template<class BE, class UT, class TH>
    void parameter(BE& be, UT& ut, const TH& th)
    {
        userland(be, ut, th);
    }
    
    template<class BE, class UT, class TH>
    void parameter(BE& be, const char* label, UT& ut, const TH& th)
    {
        kickoff(be, label, ut, th);
    }
    
} // namespace im_param

//-----------------------------------------------------------------------------
namespace im_param {
    template <class T, class U=std::false_type> struct TypeHolder {};
} // namespace im_param
    
//-----------------------------------------------------------------------------

template<class T>
struct Foo
{
    struct Bar
    {
        T val;
    };
};


namespace im_param {
    
    template<class BE>
    struct Custom<BE, int, TypeHolder<int>>
    {
        static void parameter(BE& be, int& ut, const TypeHolder<int>& th)
        {
            ut++;
        }
    };
    
    
    template<class BE, class T>
    struct Custom<BE, typename Foo<T>::Bar, TypeHolder<Foo<T>>>
    {
        static void parameter(BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th)
        {
            im_param::parameter(be, "val", ut.val, im_param::TypeHolder<int>());
        }
    };
    
} // namespace im_param
} // namespace lvl05

int im_param_foundations_level_05(int argc, char* argv[]) {

    lvl05::Backend backend{};
    lvl05::Foo<int>::Bar bar{};
    bar.val = 0;
    
    lvl05::im_param::parameter(backend, "bar", bar, lvl05::im_param::TypeHolder<lvl05::Foo<int>>());
    ASSERT(bar.val == 1);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 1);
    

    return 0;
}
    