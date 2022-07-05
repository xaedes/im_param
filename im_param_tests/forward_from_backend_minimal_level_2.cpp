#include <iostream>

#include "tests_common.h"

namespace lvl2 {

#pragma region part of backend

template<class T>
struct Forwarder
{
    template<class U>
    static void forward(U user, T& val)
    {
        forwarded(user, val);
    }
};

template<class U, class T>
void backend(U u, T& val)
{
    std::cout << "backend" << "\n";
    Forwarder<T>::forward(u, val);
}
#pragma endregion

#pragma region user code: define specializations for forwarded functions, called from backend defined above
struct MyInt
{
    int val;
};

struct MyFloat
{
    float val;
};

template<class U>
void forwarded(U u, int& val)
{
    val+=static_cast<int>(u);
}

template<class U>
void forwarded(U u, MyInt& val)
{
    val.val+=static_cast<int>(u);
}

template<class U>
void forwarded(U u, MyFloat& val)
{
    val.val*=static_cast<float>(u);
}
#pragma endregion

} // namespace lvl2

int forward_from_backend_minimal_level_2(int argc, char* argv[]) {
    // this makes it possible to forward additional templated arguments without requiring function definitions in custom types.
    
    float uf = 2.0f;
    double ud = -2.0;
    {
        lvl2::MyInt i{0};
        lvl2::backend(uf, i);
        ASSERT(i.val == 2);
    }
    {
        lvl2::MyInt i{0};
        lvl2::backend(ud, i);
        ASSERT(i.val == -2);
    }

    {
        lvl2::MyFloat f{2.0f};
        lvl2::backend(uf, f);
        ASSERT(f.val == 4.0f);
    }
    {
        lvl2::MyFloat f{2.0f};
        lvl2::backend(ud, f);
        ASSERT(f.val == -4.0f);
    }

    /*
    {
        int integer{0};
        backend(uf, integer);
        ASSERT(integer == 2);
    }

    {
        int integer{0};
        backend(ud, integer);
        ASSERT(integer == -2);
    }
    */
    return 0;
}
