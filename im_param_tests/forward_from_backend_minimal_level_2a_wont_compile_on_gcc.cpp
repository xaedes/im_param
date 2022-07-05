
// forwarded functions for fundamental types cannot be declared in user code, which is after the backend code.

#include <iostream>

#include "tests_common.h"

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

template<class U>
void forwarded(U u, int& val)
{
    val+=static_cast<int>(u);
}

#pragma endregion

int forward_from_backend_minimal_level_2a_wont_compile_on_gcc(int argc, char* argv[]) {
    // this makes it possible to forward additional templated arguments without requiring function definitions in custom types.
    
    float uf = 2.0f;
    double ud = -2.0;
    
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
    
    return 0;
}
