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
    cpp11_templates.cpp
    forward_to_backend.cpp
    # forward_from_backend_minimal.cpp
    forward_from_backend_minimal_level_0.cpp
    # forward_from_backend_minimal_level_0a_wont_compile_on_gcc.cpp
    forward_from_backend_minimal_level_1.cpp
    forward_from_backend_minimal_level_2.cpp
    # forward_from_backend_minimal_level_2a_wont_compile_on_gcc.cpp
    forward_from_backend_minimal_level_3.cpp
    forward_from_backend_minimal_level_4a.cpp
    forward_from_backend_minimal_level_4b.cpp
    forward_from_backend_minimal_level_5.cpp
)

# add the executable
add_executable (${PROJECT_NAME} ${Tests})
target_link_libraries( ${PROJECT_NAME} PRIVATE im_param )

# remove the test driver source file
set (TestsToRun ${Tests})
remove (TestsToRun ${PROJECT_NAME}.cpp)

# Add all the ADD_TEST for each test
foreach (test ${TestsToRun})
    get_filename_component (TName ${test} NAME_WE)
    add_test (NAME ${TName} COMMAND ${PROJECT_NAME} ${TName})
endforeach ()


