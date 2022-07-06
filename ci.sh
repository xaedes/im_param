#!/bin/sh

BUILD_TYPE=Release
TARGET_TRIPLET=x64-linux
TESTS_PROJECT=im_param_tests
CMAKE_GENERATOR=Ninja
# Environment variable VCPKG_FORCE_SYSTEM_BINARIES must be set to `1` on ARM and s390x platforms.
VCPKG_FORCE_SYSTEM_BINARIES=0

echo $0 $@

SCRIPT=$(which $0 2>/dev/null || realpath ./$0)
DIR=$(dirname $SCRIPT)

function_main() {
    if [ ! -z "$6" ]; then 
        VCPKG_FORCE_SYSTEM_BINARIES=$6
    fi
    if [ ! -z "$5" ]; then 
        CMAKE_GENERATOR=$5 
    fi
    if [ ! -z "$4" ]; then 
        TESTS_PROJECT=$4   
    fi
    if [ ! -z "$3" ]; then 
        TARGET_TRIPLET=$3  
    fi
    if [ ! -z "$2" ]; then 
        BUILD_TYPE=$2      
    fi

    echo ---
    echo "BUILD_TYPE:         $BUILD_TYPE"
    echo "TARGET_TRIPLET:     $TARGET_TRIPLET"
    echo "TESTS_PROJECT:      $TESTS_PROJECT"
    echo "CMAKE_GENERATOR:    $CMAKE_GENERATOR"
    echo "VCPKG_FORCE_SYSBIN: $VCPKG_FORCE_SYSTEM_BINARIES"
    echo ---
    CI_CALL="sh $0"
    ARGS="$BUILD_TYPE $TARGET_TRIPLET $TESTS_PROJECT $CMAKE_GENERATOR $VCPKG_FORCE_SYSTEM_BINARIES"

    echo "Stage: $1"
    case $1 in
        "all")        function_all;;
        "clean")      function_clean;;
        "tools")      function_tools;;
        "build")      function_build;;
        "test")       function_test;;
        "build_test") function_build_test;;
        "-")          function_build_test;;
        "help")       function_help;;
        *)            function_build_test;;
    esac
}
function_all() {
    echo "all"
    $CI_CALL clean $ARGS && $CI_CALL tools $ARGS && $CI_CALL build $ARGS && $CI_CALL test $ARGS
}
function_build_test() {
    echo "build_test"
    $CI_CALL build $ARGS && $CI_CALL test $ARGS
}
function_clean() {
    echo "Cleaning up..."
    if [ -d "$DIR/tools" ]; then
        rm -rf "$DIR/tools"
    fi
    if [ -d "$DIR/build" ]; then
        rm -rf "$DIR/build"
    fi
}
function_tools() {
    echo "Preparing tools..."
    if [ ! -d "$DIR/tools" ]; then
        mkdir -p "$DIR/tools"
    fi
    if [ ! -d "$DIR/tools/vcpkg" ]; then
        # contains patch for imgui, so that C++ 11 is enabled for imgui compilation
        git clone --depth 1 --single-branch --branch imgui-cpp11 https://github.com/xaedes/vcpkg.git "$DIR/tools/vcpkg/"
        cd "$DIR/tools/vcpkg/"
        git pull origin imgui-cpp11
        cd "$DIR/"
        # git clone https://github.com/microsoft/vcpkg.git "$DIR/tools/vcpkg/"
        "$DIR/tools/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
    fi
}
function_build() {
    echo "Building..."
    export VCPKG_FEATURE_FLAGS=versions
    export VCPKG_TARGET_TRIPLET=$TARGET_TRIPLET
    if [ $VCPKG_FORCE_SYSTEM_BINARIES -eq 1 ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=$VCPKG_FORCE_SYSTEM_BINARIES
    fi
    mkdir -p "$DIR/build/Linux/$TARGET_TRIPLET/$BUILD_TYPE" || true
    cd "$DIR/build/Linux/$TARGET_TRIPLET/$BUILD_TYPE" 
    pwd
    echo cmake --version
    cmake --version
    echo cmake -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=$DIR/tools/vcpkg/scripts/buildsystems/vcpkg.cmake "$DIR"
    cmake -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=$DIR/tools/vcpkg/scripts/buildsystems/vcpkg.cmake "$DIR"
    cd "$DIR"
    cmake --build "$DIR/build/Linux/$TARGET_TRIPLET/$BUILD_TYPE"
}
function_test() {
    echo "Testing..."
    cd "$DIR/build/Linux/$TARGET_TRIPLET/$BUILD_TYPE/$TESTS_PROJECT/"
    echo ctest --version
    ctest --version
    echo ctest
    ctest
    cd "$DIR"
}
function_help() {
    echo "Usage:"
    echo "$CI_CALL [all|clean|tools|build|test|build_test|-|help]"
}

function_main $@
