#include <iostream>

#include "tests_common.h"

#pragma region part of backend
template<class T>
void forwarded(T& val);

template<class T>
void backend(T& val)
{
    std::cout << "backend" << "\n";
    forwarded(val);
}
#pragma endregion

#pragma region user code: define specializations for forwarded functions, called from backend defined above
template<>
void forwarded(int& val)
{
    ++val;
}

template<>
void forwarded(float& val)
{
    val*=2;
}
#pragma endregion

int forward_from_backend_minimal_level_0(int argc, char* argv[]) {
    // calling the backend is possible because forwarded specializations are defined
    int i=1;
    backend(i);
    ASSERT(i == 2);

    float f=2;
    backend(f);
    ASSERT(f == 4);

    return 0;
}


