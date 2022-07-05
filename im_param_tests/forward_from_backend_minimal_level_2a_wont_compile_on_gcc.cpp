
// forwarded functions for fundamental types cannot be declared in user code, which is after the backend code.

// [1/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/im_param_tests.cpp.o
// [2/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_0.cpp.o
// [3/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp.o
// FAILED: im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp.o 
// /usr/bin/c++   -I../../../../im_param/include -isystem vcpkg_installed/x64-linux/include -O3 -DNDEBUG -MD -MT im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp.o -MF im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp.o.d -o im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp.o -c ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp: In instantiation of 'static void Forwarder<T>::forward(U, T&) [with U = float; T = int]':
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:24:26:   required from 'void backend(U, T&) [with U = float; T = int]'
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:46:28:   required from here
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:16:18: error: 'forwarded' was not declared in this scope, and no declarations were found by argument-dependent lookup at the point of instantiation [-fpermissive]
//    16 |         forwarded(user, val);
//       |         ~~~~~~~~~^~~~~~~~~~~
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:31:6: note: 'template<class U> void forwarded(U, int&)' declared here, later in the translation unit
//    31 | void forwarded(U u, int& val)
//       |      ^~~~~~~~~
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp: In instantiation of 'static void Forwarder<T>::forward(U, T&) [with U = double; T = int]':
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:24:26:   required from 'void backend(U, T&) [with U = double; T = int]'
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:52:28:   required from here
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:16:18: error: 'forwarded' was not declared in this scope, and no declarations were found by argument-dependent lookup at the point of instantiation [-fpermissive]
//    16 |         forwarded(user, val);
//       |         ~~~~~~~~~^~~~~~~~~~~
// ../../../../im_param_tests/forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp:31:6: note: 'template<class U> void forwarded(U, int&)' declared here, later in the translation unit
//    31 | void forwarded(U u, int& val)
//       |      ^~~~~~~~~
// [4/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_1.cpp.o
// [5/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_2.cpp.o
// [6/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/cpp11_templates.cpp.o
// [7/8] Building CXX object im_param_tests/CMakeFiles/im_param_tests.dir/forward_to_backend.cpp.o

#include <iostream>

#include "tests_common.h"

namespace lvl2a {

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


} // namespace lvl2a

int forward_from_backend_minimal_level_2a_wont_compile_on_gcc(int argc, char* argv[]) {

    // this makes it possible to forward additional templated arguments without requiring function definitions in custom types.
    
    float uf = 2.0f;
    double ud = -2.0;
    
    {
        int integer{0};
        lvl2a::backend(uf, integer);
        ASSERT(integer == 2);
    }

    {
        int integer{0};
        lvl2a::backend(ud, integer);
        ASSERT(integer == -2);
    }
    
    return 0;
}
