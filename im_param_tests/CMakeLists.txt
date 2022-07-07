cmake_minimum_required(VERSION 3.8)

project( im_param_tests )

enable_testing()
include(CTest)

# create the testing file and list of tests
create_test_sourcelist (Tests
    ${PROJECT_NAME}.cpp
    # add tests here
    # simple_struct.cpp
    # multi_base_value.cpp
    # generic_and_nested_types.cpp
    forward_to_backend.cpp
    # forward_from_backend_minimal.cpp
    im_param_foundations_level_00.cpp
    # im_param_foundations_level_00a_wont_compile_on_gcc.cpp
    im_param_foundations_level_01.cpp
    im_param_foundations_level_02.cpp
    # im_param_foundations_level_02a_wont_compile_on_gcc.cpp
    im_param_foundations_level_03.cpp
    im_param_foundations_level_04a.cpp
    im_param_foundations_level_04b.cpp
    im_param_foundations_level_05.cpp
    im_param_foundations_level_06.cpp
    im_param_foundations_level_07.cpp
    im_param_foundations_level_08.cpp
    im_param_foundations_level_09.cpp
    im_param_foundations_level_10.cpp
)

# add the executable
add_executable (${PROJECT_NAME} ${Tests})
target_link_libraries( ${PROJECT_NAME} PRIVATE im_param )

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)

# remove the test driver source file
set (TestsToRun ${Tests})
remove (TestsToRun ${PROJECT_NAME}.cpp)

# Add all the ADD_TEST for each test
foreach (test ${TestsToRun})
    get_filename_component (TName ${test} NAME_WE)
    add_test (NAME ${TName} COMMAND ${PROJECT_NAME} ${TName})
endforeach ()


