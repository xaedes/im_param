
#include <type_traits>

#include "tests_common.h"

template<bool C, class T>
using enable_if_t = typename std::enable_if<C,T>::type;

template<
    class T,
    enable_if_t<std::is_integral<T>::value, bool> = true
>
bool is_int(T val)
{
    return true;
}

template<
    class T,
    enable_if_t<std::is_floating_point<T>::value, bool> = true
>
bool is_int(T val)
{
    return false;
}

int cpp11_templates(int argc, char* argv[])
{
    int i=1;
    float f=2.0f;
    ASSERT(is_int(i) == true);
    ASSERT(is_int(f) == false);
    return 0;
}
