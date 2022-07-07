#include <iostream>
#include <string>
#include <type_traits>

#include "tests_common.h"

// add additional templated Args to type-holder-functions

namespace lvl08 {

namespace im_param {

    template<bool C, class T>
    using enable_if_t = typename std::enable_if<C,T>::type;

    template<bool C, class A, class B>
    using conditional_t = typename std::conditional<C,A,B>::type;

    template<class T>
    using is_fundamental_type = std::is_integral<T>;

} // namespace im_param

//-----------------------------------------------------------------------------

namespace im_param {
    template <class T, class U=std::false_type> struct TypeHolder {};

    template <class T>
    struct is_type_holder : public std::false_type {};

    template <class T>
    struct is_type_holder<TypeHolder<T>> : public std::true_type {};

} // namespace im_param
    
//-----------------------------------------------------------------------------

namespace im_param {

    template <class T> 
    using enable_if_fundamental = enable_if_t<is_fundamental_type<T>::value, bool>;

    template <class T> 
    using enable_if_type_holder = enable_if_t<is_type_holder<T>::value, bool>;

} // namespace im_param
    
//-----------------------------------------------------------------------------

namespace im_param {
    
    template<class BE, class UT, class TH>
    struct Custom
    {
        static_assert(is_type_holder<TH>::value == true, "is_type_holder<TH>::value == true");
        template<class... Args>
        static void parameter(BE& be, UT& ut, const TH& th, Args&&... args) 
        { 
            std::cout << "Custom<BE,UT,TH>::parameter(BE& be, UT& ut, const TH& th, Args&&... args) " << "\n";
            /* empty */ 
        }
    };
    
    // default UserLand implementation will forward to Custom::parameter synonyms.
    // UserLand<BE,T> can be specialized for T by users or they specialize Custom<BE,T>.
    template<class BE, class UT, class TH>
    struct UserLand
    {
        static_assert(is_type_holder<TH>::value == true, "is_type_holder<TH>::value == true");
        template<class... Args>
        static void specialization(BE& be, UT& ut, const TH& th, Args&&... args) 
        {
            std::cout << "UserLand<BE,UT,TH>::specialization(BE& be, UT& ut, const TH& th, Args&&... args) " << "\n";
            Custom<BE,UT,TH>::parameter(be, ut, th, std::forward<Args>(args)...);
        }
    };

    // go into userland code
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void userland(BE& be, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "userland(BE& be, UT& ut, const TH& th, Args&&... args)" << "\n";
        UserLand<BE,UT,TH>::specialization(be, ut, th, std::forward<Args>(args)...);
    }
    
    // kick off the call chain by going into the backend BE
    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void kickoff_fundamental(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "kickoff_fundamental(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";
        be.work_backend_fundamental(label, ut, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void kickoff(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "kickoff(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";
        be.work_backend(label, ut, th, std::forward<Args>(args)...);
    }

} // namespace im_param

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{
    int called_with_int_fundamental = 0;
    int called_with_ut = 0;

    template<class... Args>
    void work_backend_fundamental(const char* label, int& ut, int min=0, int max=10, Args&&... args)
    {
        std::cout << "Backend::work_backend_fundamental('" << label << "', " << ut << ", ...)" << "\n";
        if (ut < min) ut = min;
        if (ut > max) ut = max;
        ++called_with_int_fundamental;
    }

    
    template<class UT, class TH, im_param::enable_if_type_holder<TH> = true, class... Args>
    void work_backend(const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "Backend::work_backend('" << label << "', ut, ...)" << "\n";
        ++called_with_ut;
        im_param::userland(*this, ut, th, std::forward<Args>(args)...);
    }
};

//-----------------------------------------------------------------------------

namespace im_param {

    // `parameter` synonyms

    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void parameter(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";

        kickoff_fundamental(be, label, ut, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void parameter(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";

        kickoff(be, label, ut, th, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void parameter(BE& be, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, UT& ut, const TH& th, Args&&... args)" << "\n";

        userland(be, ut, th, std::forward<Args>(args)...);
    }

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
        static void parameter(
            BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th, 
            const typename Foo<T>::Bar& min,
            const typename Foo<T>::Bar& max
        )
        {
            std::cout << "Custom::parameter(BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th, const typename Foo<T>::Bar& min, const typename Foo<T>::Bar& max)" << "\n";
            // implicit fundamental type
            im_param::parameter(be, "foo", ut.foo, min.foo, max.foo);
            im_param::parameter(be, "bar", ut.bar, min.bar, max.bar);
            // explicit fundamental type
            im_param::kickoff_fundamental(be, "baz", ut.baz, min.baz, max.baz);

        }
    };
    
} // namespace im_param
} // namespace lvl08

int im_param_foundations_level_08(int argc, char* argv[]) {

    lvl08::Backend backend{};
    lvl08::Foo<int>::Bar bar{};
    lvl08::Foo<int>::Bar min{};
    lvl08::Foo<int>::Bar max{};
    bar.foo = -200;
    bar.bar = +200;
    bar.baz = +200;

    min.foo = -100;
    min.bar = -100;
    min.baz =  -10;

    max.foo = +100;
    max.bar = +100;
    max.baz =  +10;
    lvl08::im_param::parameter(backend, "bar", bar, lvl08::im_param::TypeHolder<lvl08::Foo<int>>(), min, max);
    ASSERT(bar.foo == -100);
    ASSERT(bar.bar == +100);
    ASSERT(bar.baz == +10);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int_fundamental == 3);
    

    return 0;
}
    