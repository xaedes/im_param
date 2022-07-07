#include <iostream>
#include <string>
#include <type_traits>

#include "tests_common.h"

// add fundamental_array and fundamental_ndarray

namespace lvl10 {

namespace im_param {

    template<bool C, class T>
    using enable_if_t = typename std::enable_if<C,T>::type;

    template<bool C, class A, class B>
    using conditional_t = typename std::conditional<C,A,B>::type;

    template<class...> struct conjunction : std::true_type { };
    template<class B1> struct conjunction<B1> : B1 { };
    template<class B1, class... Bn>
    struct conjunction<B1, Bn...> : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

    template<class T>
    using is_fundamental_type = conjunction<std::is_integral<T>>;

    template<class T>
    using is_size = conjunction<std::is_integral<T>>;


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

    template <class T> 
    using enable_if_size = enable_if_t<is_size<T>::value, bool>;

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
    template<
        class BE, class UT, class TH, 
        enable_if_type_holder<TH> = true, 
        class... Args
    >
    void map(BE& be, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "map(BE& be, UT& ut, const TH& th, Args&&... args)" << "\n";
        UserLand<BE,UT,TH>::map(be, ut, th, std::forward<Args>(args)...);
    }
    
    // kick off the call chain by going into the backend BE
    template<
        class BE, class UT, 
        enable_if_fundamental<UT> = true, 
        class... Args
    >
    void map_item_fundamental(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "map_item_fundamental(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";
        be.map_item_fundamental(label, ut, std::forward<Args>(args)...);
    }

    template<
        class BE, class UT, class S, 
        enable_if_size<S> = true, 
        enable_if_fundamental<UT> = true, 
        class... Args>
    void map_item_fundamental_array(BE& be, const char* label, UT* ut, S size, Args&&... args)
    {
        std::cout << "map_item_fundamental_array(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";
        be.map_item_fundamental_array(label, ut, size, std::forward<Args>(args)...);
    }

    template<
        class BE, class UT, class S, 
        enable_if_size<S> = true, 
        enable_if_fundamental<UT> = true, 
        class... Args
    >
    void map_item_fundamental_ndarray(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)
    {
        std::cout << "map_item_fundamental_ndarray(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)" << "\n";
        be.map_item_fundamental_ndarray(label, ut, shape, ndim, std::forward<Args>(args)...);
    }

    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void map_item_map(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "map_item_map(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";
        be.map_item_map(label, ut, th, std::forward<Args>(args)...);
    }

} // namespace im_param


//-----------------------------------------------------------------------------
namespace im_param {

    template<class T, class PV>
    struct PointerOrValue
    {
        PointerOrValue() = delete;
    };

    template<class T>
    struct PointerOrValue<T, T>
    {
        PointerOrValue() = delete;
        PointerOrValue(T v) : v(v) { }
        T v;
        template<class S>
        const T& operator[](S idx) { return v; }
        const T& operator*() { return v; }
    };

    template<class T>
    struct PointerOrValue<T, const T*>
    {
        PointerOrValue() = delete;
        PointerOrValue(const T* p) : p(p) { }
        const T* p;
        template<class S>
        const T& operator[](S idx) { return p[idx]; }
        const T& operator*() { return p[0]; }
    };

    template<class T>
    struct PointerOrValue<T, T*>
    {
        PointerOrValue() = delete;
        PointerOrValue(T* p) : p(p) { }
        T* p;
        template<class S>
        T& operator[](S idx) { return p[idx]; }
        T& operator*() { return p[0]; }
    };
    
    template<class T, class PV>
    PointerOrValue<T,PV> make_pv(PV pv) 
    { return PointerOrValue<T,PV>(pv); }

} // namespace im_param
//-----------------------------------------------------------------------------

// backends may call userland code wrapped with backend specific work.
// the backend implementation can decide which fundamental types it handles itself without 
// recursion to userland code.
struct Backend
{

    template<
        class UT, 
        im_param::enable_if_fundamental<UT> = true, 
        class... Args
    >
    void map_item_fundamental(const char* label, UT& ut, UT min=0, UT max=10, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental('" << label << "', " << ut << ", ...)" << "\n";
        if (ut < min) ut = min;
        if (ut > max) ut = max;
    }

    template<
        class UT, class S, 
        im_param::enable_if_fundamental<UT> = true, 
        im_param::enable_if_size<S> = true, 
        class... Args
    >
    void map_item_fundamental_array(const char* label, UT* ut, S size, UT min=0, UT max=10, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental_array('" << label << "', ut["<< size <<"], ...)" << "\n";
        for (S i=0; i<size; ++i)
        {
            if (ut[i] < min) ut[i] = min;
            if (ut[i] > max) ut[i] = max;
        }
    }


    template<
        class UT, class S, 
        im_param::enable_if_fundamental<UT> = true, 
        im_param::enable_if_size<S> = true, 
        class... Args
    >
    void map_item_fundamental_array(const char* label, UT* ut, S size, const UT* min=nullptr, const UT* max=nullptr, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental_array('" << label << "', ut["<< size <<"], ...)" << "\n";
        if (min != nullptr)
        for (S i=0; i<size; ++i)
        {
            if (ut[i] < min[i]) ut[i] = min[i];
        }

        if (max != nullptr)
        for (S i=0; i<size; ++i)
        {
            if (ut[i] > max[i]) ut[i] = max[i];
        }
    }


    template<
        class UT, class S,
        im_param::enable_if_fundamental<UT> = true, 
        im_param::enable_if_size<S> = true, 
        class... Args
    >
    void map_item_fundamental_ndarray(const char* label, UT* ut, const S* shape, S ndim, UT min=0, UT max=10, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental_ndarray('" << label << ", ...)" << "\n";
        S total_size = 1;
        for(S d=0; d<ndim; ++d)
        {
            total_size *= shape[d];
        }
        for(S i=0; i<total_size; ++i)
        {
            if (ut[i] < min) ut[i] = min;
            if (ut[i] > max) ut[i] = max;
        }
    }


    template<
        class UT, class S,
        im_param::enable_if_fundamental<UT> = true, 
        im_param::enable_if_size<S> = true, 
        class... Args
    >
    void map_item_fundamental_ndarray(const char* label, UT* ut, const S* shape, S ndim, const UT* min=nullptr, const UT* max=nullptr, Args&&... args)
    {
        std::cout << "Backend::map_item_fundamental_ndarray('" << label << ", ...)" << "\n";
        S total_size = 1;
        for(S d=0; d<ndim; ++d)
        {
            total_size *= shape[d];
        }
        if (min != nullptr)
        for (S i=0; i<total_size; ++i)
        {
            if (ut[i] < min[i]) ut[i] = min[i];
        }

        if (max != nullptr)
        for (S i=0; i<total_size; ++i)
        {
            if (ut[i] > max[i]) ut[i] = max[i];
        }

    }

    
    template<class UT, class TH, im_param::enable_if_type_holder<TH> = true, class... Args>
    void map_item_map(const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "Backend::map_item_map('" << label << "', ut, ...)" << "\n";
        im_param::map(*this, ut, th, std::forward<Args>(args)...);
    }


};

//-----------------------------------------------------------------------------

namespace im_param {

    // `map_item` synonyms

    template<class BE, class UT, enable_if_fundamental<UT> = true, class... Args>
    void map_item(BE& be, const char* label, UT& ut, Args&&... args)
    {
        std::cout << "im_param::map_item(BE& be, const char* label, UT& ut, Args&&... args)" << "\n";

        map_item_fundamental(be, label, ut, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class S, enable_if_fundamental<UT> = true, enable_if_size<S> = true, class... Args>
    void map_item(BE& be, const char* label, UT* ut, S size, Args&&... args)
    {
        std::cout << "im_param::map_item(BE& be, const char* label, UT* ut, S size, Args&&... args)" << "\n";

        map_item_fundamental_array(be, label, ut, size, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class S, enable_if_fundamental<UT> = true, enable_if_size<S> = true, class... Args>
    void map_item(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)
    {
        std::cout << "im_param::map_item(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)" << "\n";

        map_item_fundamental_ndarray(be, label, ut, shape, ndim, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class TH, enable_if_type_holder<TH> = true, class... Args>
    void map_item(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)
    {
        std::cout << "im_param::map_item(BE& be, const char* label, UT& ut, const TH& th, Args&&... args)" << "\n";

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
    
    template<class BE, class UT, class S, enable_if_fundamental<UT> = true, enable_if_size<S> = true, class... Args>
    void parameter(BE& be, const char* label, UT* ut, S size, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT* ut, S size, Args&&... args)" << "\n";

        map_item_fundamental_array(be, label, ut, size, std::forward<Args>(args)...);
    }
    
    template<class BE, class UT, class S, enable_if_fundamental<UT> = true, enable_if_size<S> = true, class... Args>
    void parameter(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)
    {
        std::cout << "im_param::parameter(BE& be, const char* label, UT* ut, const S* shape, S ndim, Args&&... args)" << "\n";

        map_item_fundamental_ndarray(be, label, ut, shape, ndim, std::forward<Args>(args)...);
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
struct Vec3
{
    T x = 0;
    T y = 0;
    T z = 0;
};

template<class T>
struct Foo
{
    struct Bar
    {
        T foo[3] = {0,0,0};
        T bar[3] = {0,0,0};
        T baz[3] = {0,0,0};
        T fob[3] = {0,0,0};
        Vec3<T> vec3{};

        T mat3[16] = {
            1,0,0,
            0,1,0,
            0,0,1
        };
    };
};

namespace im_param {
    

    template<class BE, class T>
    struct UserLand<BE, typename Foo<T>::Bar, TypeHolder<Foo<T>>>
    {
        static void map(
            BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th, 
            const typename Foo<T>::Bar& min,
            const typename Foo<T>::Bar& max
        )
        {
            std::cout << "Custom::parameter(BE& be, typename Foo<T>::Bar& ut, const TypeHolder<Foo<T>>& th, const typename Foo<T>::Bar& min, const typename Foo<T>::Bar& max)" << "\n";
            // implicit fundamental type
            im_param::parameter(be, "foo", ut.foo, 3, min.foo, max.foo);
            // explicit map_item of fundamental type
            im_param::map_item_fundamental_array(be, "bar", ut.bar, 3, min.bar, max.bar);
            // we can also use the map_item synonym
            im_param::map_item(be, "baz", ut.baz, 3, min.baz, max.baz);
            // scalar values can be used as min and max instead of pointers
            im_param::map_item(be, "fob", ut.fob, 3, -50, +50);
            
            im_param::map_item(be, "vec3", &ut.vec3.x, 3, -25, +25);

            int shape[2]{3,3};
            im_param::map_item(be, "mat3", ut.mat3, shape, 2, -10, +10);

        }
    };
    
} // namespace im_param
} // namespace lvl10

int im_param_foundations_level_10(int argc, char* argv[]) {

    lvl10::Backend backend{};
    lvl10::Foo<int>::Bar bar{};
    lvl10::Foo<int>::Bar min{};
    lvl10::Foo<int>::Bar max{};

    auto SetValues = [](
        auto& bar, 
        auto val_x, auto val_y, auto val_z
    )
    {
        bar.foo[0] = val_x;
        bar.foo[1] = val_y;
        bar.foo[2] = val_z;

        bar.bar[0] = val_x;
        bar.bar[1] = val_y;
        bar.bar[2] = val_z;

        bar.baz[0] = val_x;
        bar.baz[1] = val_y;
        bar.baz[2] = val_z;
        
        bar.fob[0] = val_x;
        bar.fob[1] = val_y;
        bar.fob[2] = val_z;
        
        bar.vec3.x = val_x;
        bar.vec3.y = val_y;
        bar.vec3.z = val_z;
        
        bar.mat3[0*3+0] = val_x;
        bar.mat3[1*3+1] = val_y;
        bar.mat3[2*3+2] = val_z;
    };

    SetValues(bar, -1000, 0, +1000);
    SetValues(min, -300, -200, -100);
    SetValues(max, +100, +200, +300);

    lvl10::im_param::parameter(backend, "bar", bar, lvl10::im_param::TypeHolder<lvl10::Foo<int>>(), min, max);
    
    ASSERT(bar.foo[0] == -300);
    ASSERT(bar.foo[0] == bar.bar[0]);
    ASSERT(bar.foo[0] == bar.baz[0]);

    ASSERT(bar.foo[1] == 0);
    ASSERT(bar.foo[1] == bar.bar[1]);
    ASSERT(bar.foo[1] == bar.baz[1]);

    ASSERT(bar.foo[2] == +300);
    ASSERT(bar.foo[2] == bar.bar[2]);
    ASSERT(bar.foo[2] == bar.baz[2]);

    ASSERT(bar.fob[0] == -50);
    ASSERT(bar.fob[1] == 0);
    ASSERT(bar.fob[2] == +50);

    ASSERT(bar.vec3.x == -25);
    ASSERT(bar.vec3.y == 0);
    ASSERT(bar.vec3.z == +25);

    ASSERT(bar.mat3[0*3+0] == -10);
    ASSERT(bar.mat3[1*3+1] == 0);
    ASSERT(bar.mat3[2*3+2] == +10);

    return 0;
}
    