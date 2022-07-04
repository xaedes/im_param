#include <iostream>

#include "tests_common.h"

#pragma region part of backend
template<class U, class T>
void backend(U u, T& val)
{
    std::cout << "backend" << "\n";
    val.forwarded(u);
}
#pragma endregion

#pragma region user code: define specializations for forwarded functions, called from backend defined above
struct MyInt
{
    int val;
    template<class U>
    void forwarded(U u)
    {
        val+=static_cast<decltype(val)>(u);
    }
};

struct MyFloat
{
    float val;
    template<class U>
    void forwarded(U u)
    {
        val*=static_cast<decltype(val)>(u);
    }
};
#pragma endregion

int forward_from_backend_minimal_level_1(int argc, char* argv[]) {
    // this makes it possible to forward additional templated arguments.
    // a big disadvantage  in this formulation is that it requires functional definitions in the custom types.
    // the idea of im_param is that you can provide definitions for any custom type, whether 
    // you have control over its code or not.
    
    float uf = 2.0f;
    double ud = -2.0;
    {
        MyInt i{0};
        backend(uf, i);
        ASSERT(i.val == 2);
    }
    {
        MyInt i{0};
        backend(ud, i);
        ASSERT(i.val == -2);
    }

    {
        MyFloat f{2.0f};
        backend(uf, f);
        ASSERT(f.val == 4.0f);
    }
    {
        MyFloat f{2.0f};
        backend(ud, f);
        ASSERT(f.val == -4.0f);
    }

    return 0;
}
