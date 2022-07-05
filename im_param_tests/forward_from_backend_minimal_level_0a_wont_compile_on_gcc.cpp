
//  Adding another template class won't work resulting in 
//  "undefined reference to `void forwarded<float, int>(float, int)'" error 
//  or similar on gcc:

#include <iostream>
#include <type_traits>

template<class U, class T>
void forwarded(U user, T val);

template<class U, class T>
void backend(U user, T val)
{
    std::cout << "backend" << "\n";
    forwarded(user, val);
}

template<class U>
void forwarded(U user, int val)
{
    std::cout << "forwarded" << user << "\n";
}

int forward_from_backend_minimal_level_0a_wont_compile_on_gcc(int argc, char* argv[]) {
    float u;
    int i=1;
    backend(u, i);

    return 0;
}
 