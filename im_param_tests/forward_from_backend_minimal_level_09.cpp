#include <iostream>
#include <string>
#include <type_traits>

#include "tests_common.h"

// rename functions according to im_param data grammar semantics
// see doc/grammar-1.0.txt

namespace lvl09 {

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
        static void map(BE& be, UT& ut, const TH& th, Args&&... args) 
        {
            std::cout << "UserLand<BE,UT,TH>::map(BE& be, UT& ut, const TH& th, Args&&... args) " << "\n";
            Custom<BE,UT,TH>::parameter(be, ut, th, std::forward<Args>(args)...);
        }
    };

    // go into userland code
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void map(BE& be, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "map(BE& be, UT& ut, const TH& th, Args&&... args)" << "\n";
        UserLand<BE,UT,TH>::map(be, ut, th, std::forward<Args>(args)...);
    }
    
    // kick off the call chain by going into the backend BE
    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void map_item_fundamental(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "map_item_fundamental(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";
        be.map_item_fundamental(label, ut, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void map_item_map(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "map_item_map(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";
        be.map_item_map(label, ut, th, std::forward<Args>(args)...);
    }

} // namespace im_param


//-----------------------------------------------------------------------------

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{
    int called_with_int_fundamental = 0;
    int called_with_ut = 0;

    template<class... Args>
    void map_item_fundamental(const char* label, int& ut, int min=0, int max=10, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental('" << label << "', " << ut << ", ...)" << "\n";
        if (ut < min) ut = min;
        if (ut > max) ut = max;
        ++called_with_int_fundamental;
    }

    
    template<class UT, class TH, im_param::enable_if_type_holder<TH> = true, class... Args>
    void map_item_map(const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "Backend::map_item_map('" << label << "', ut, ...)" << "\n";
        ++called_with_ut;
        im_param::map(*this, ut, th, std::forward<Args>(args)...);
    }
};

//-----------------------------------------------------------------------------

namespace im_param {

    // `map_item` synonyms

    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void map_item(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";

        map_item_fundamental(be, label, ut, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void map_item(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";

        map_item_map(be, label, ut, th, std::forward<Args>(args)...);
    }

} // namespace im_param

//-----------------------------------------------------------------------------

namespace im_param {

    // `parameter` synonyms

    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void parameter(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";

        map_item_fundamental(be, label, ut, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void parameter(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";

        map_item_map(be, label, ut, th, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void parameter(BE& be, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, UT& ut, const TH& th, Args&&... args)" << "\n";

        map(be, ut, th, std::forward<Args>(args)...);
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
            // explicit map_item of fundamental type
            im_param::map_item_fundamental(be, "bar", ut.bar, min.bar, max.bar);
            // we can also use the map_item synonym
            im_param::map_item(be, "baz", ut.baz, min.baz, max.baz);

        }
    };
    
} // namespace im_param
} // namespace lvl09

int forward_from_backend_minimal_level_09(int argc, char* argv[]) {

    lvl09::Backend backend{};
    lvl09::Foo<int>::Bar bar{};
    lvl09::Foo<int>::Bar min{};
    lvl09::Foo<int>::Bar max{};
    bar.foo = -200;
    bar.bar = +200;
    bar.baz = +200;

    min.foo = -100;
    min.bar = -100;
    min.baz =  -10;

    max.foo = +100;
    max.bar = +100;
    max.baz =  +10;
    lvl09::im_param::parameter(backend, "bar", bar, lvl09::im_param::TypeHolder<lvl09::Foo<int>>(), min, max);
    ASSERT(bar.foo == -100);
    ASSERT(bar.bar == +100);
    ASSERT(bar.baz == +10);
    ASSERT(backend.called_with_ut == 1);
    ASSERT(backend.called_with_int_fundamental == 3);
    

    return 0;
}
    