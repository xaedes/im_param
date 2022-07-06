#include <iostream>
#include <string>
#include <type_traits>

#include "tests_common.h"

// add additional templated Args to fundamental-type-functions
// these can e.g. be used to pass min and max values to the backend.

namespace lvl7 {
namespace im_param {

    template<bool C, class T>
    using enable_if_t = typename std::enable_if<C,T>::type;

    template<bool C, class A, class B>
    using conditional_t = typename std::conditional<C,A,B>::type;

    template<class T>
    using is_fundamental_type = std::is_integral<T>;

} // namespace im_param

namespace im_param {
    
    template<class BE, class UT, class TH>
    struct Custom
    {
        static void parameter(BE& be, UT& ut, const TH& th) 
        { 
            std::cout << "Custom<BE,UT,TH>::parameter(BE& be, UT& ut, const TH& th) " << "\n";
            /* empty */ 
        }
    };
    
    // default UserLand implementation will forward to Custom::parameter synonyms.
    // UserLand<BE,T> can be specialized for T by users or they specialize Custom<BE,T>.
    template<class BE, class UT, class TH>
    struct UserLand
    {
        static void spezialization(BE& be, UT& ut, const TH& th) 
        {
            std::cout << "UserLand<BE,UT,TH>::spezialization(BE& be, UT& ut, const TH& th) " << "\n";
            Custom<BE,UT,TH>::parameter(be, ut, th);
        }
    };

    // go into userland code
    template<class BE, class UT, class TH>
    void userland(BE& be, UT& ut, const TH& th)
    {
        std::cout << "userland(BE& be, UT& ut, const TH& th)" << "\n";
        UserLand<BE,UT,TH>::spezialization(be, ut, th);
    }
    
    // kick off the call chain by going into the backend BE
    template<class BE, class UT, enable_if_t<is_fundamental_type<UT>::value, bool> = true, class... Args>
    void kickoff_fundamental(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "kickoff_fundamental(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";
        be.work_backend_fundamental(label, ut, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH>
    void kickoff(BE& be, const char* label, UT& ut, const TH& th)
    {
        std::cout << "kickoff(BE& be, const char* label, UT& ut, const TH& th)" << "\n";
        be.work_backend(label, ut, th);
    }

} // namespace im_param

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{
    int called_with_int_fundamental = 0;
    int called_with_int = 0;
    int called_with_ut = 0;

    void work_backend_fundamental(const char* label, int& ut, int min=0, int max=10)
    {
        std::cout << "Backend::work_backend_fundamental('" << label << "', " << ut << ")" << "\n";
        if (ut < min) ut = min;
        if (ut > max) ut = max;
        ++called_with_int_fundamental;
    }

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

    template<class BE, class UT, enable_if_t<is_fundamental_type<UT>::value, bool> = true, class... Args>
    void parameter(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";

        kickoff_fundamental(be, label, ut, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class TH>
    void parameter(BE& be, const char* label, UT& ut, const TH& th)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, const TH& th)" << "\n";

        kickoff(be, label, ut, th);
    }

    template<class BE, class UT, class TH>
    void parameter(BE& be, UT& ut, const TH& th)
    {
        std::cout << "im_param::parameter(BE& be, UT& ut, const TH& th)" << "\n";

        userland(be, ut, th);
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
        T foo = 0;
        T bar = 0;
        T baz = 0;
    };
};

namespace im_param {
    

    template<class BE, class T>
    struct Custom<BE, typename Foo<T>::Bar, TypeHolder<Foo<T>>>
    {
        static void parameter(BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th)
        {
            std::cout << "Custom::parameter(BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th)" << "\n";
            // implicit fundamental type
            im_param::parameter(be, "foo", ut.foo, -100, +100);
            // explicit fundamental type
            im_param::kickoff_fundamental(be, "bar", ut.bar, -100, +100);
            im_param::kickoff_fundamental(be, "baz", ut.baz);

        }
    };
    
} // namespace im_param
} // namespace lvl7

int forward_from_backend_minimal_level_7(int argc, char* argv[]) {

    lvl7::Backend backend{};
    lvl7::Foo<int>::Bar bar{};
    bar.foo = -200;
    bar.bar = +200;
    bar.baz = +200;
    lvl7::im_param::parameter(backend, "bar", bar, lvl7::im_param::TypeHolder<lvl7::Foo<int>>());
    ASSERT(bar.foo == -100);
    ASSERT(bar.bar == +100);
    ASSERT(bar.baz == +10);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int == 0);
    ASSERT(backend.called_with_int_fundamental == 3);
    

    return 0;
}
    