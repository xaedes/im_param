
// Adding another template class won't work resulting in linkage errors on some platforms.
// It does work for windows with Visual Studio.
// 
// Linkage error on ubuntu:focal and other gcc:
// 
//  > /usr/bin/ld: im_param_tests/CMakeFiles/im_param_tests.dir/forward_from_backend_minimal_level_0a_wont_compile_on_gcc.cpp.o: in function `forward_from_backend_minimal_level_0a_wont_compile_on_gcc(int, char**)':
//  > forward_from_backend_minimal_level_0a_wont_compile_on_gcc.cpp:(.text+0x42): undefined reference to `void forwarded<float, int>(float, int)'
//  > collect2: error: ld returned 1 exit status
//


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

// this link clarifies a LOT: http://www.gotw.ca/publications/mill17.htm
//
// "there's no such thing as a partial specialization of a function template!""
//
// Finally, let's focus on function templates only and consider the overloading
// rules to see which ones get called in different situations. The rules are
// pretty simple, at least at a high level, and can be expressed as a classic
// two-class system:
//  - Nontemplate functions are first-class citizens. A plain old nontemplate
//    function that matches the parameter types as well as any function template
//    will be selected over an otherwise-just-as-good function template.
//  - If there are no first-class citizens to choose from that are at least as
//    good, then function base templates as the second-class citizens get
//    consulted next. Which function base template gets selected depends on
//    which matches best and is the "most specialized" (important note: this use
//    of "specialized" oddly enough has nothing to do with template
//    specializations; it's just an unfortunate colloquialism) according to a
//    set of fairly arcane rules:
//    - If it's clear that there's one "most specialized" function base
//      template, that one gets used. If that base template happens to be
//      specialized for the types being used, the specialization will get used,
//      otherwise the base template instantiated with the correct types will be
//      used.
//    - Else if there's a tie for the "most specialized" function base template,
//      the call is ambiguous because the compiler can't decide which is a
//      better match. The programmer will have to do something to qualify the
//      call and say which one is wanted.
//    - Else if there's no function base template that can be made to match, the
//      call is bad and the programmer will have to fix the code.
//
// The key to understanding this is simple, and here it is: Specializations
// don't overload.
//
// Overload resolution only selects a base template (or a nontemplate function,
// if one is available). Only after it's been decided which base template is
// going to be selected, and that choice is locked in, will the compiler look
// around to see if there happens to be a suitable specialization of that
// template available, and if so that specialization will get used.
// 