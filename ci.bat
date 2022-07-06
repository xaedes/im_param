@echo off

setlocal

set BUILD_TYPE=Release
set TARGET_TRIPLET=x64-windows
set TESTS_PROJECT=im_param_tests
set CMAKE_GENERATOR=Ninja
set DIR=%~dp0

rem Ensure that your Jenkins Windows Agent has PATH set up 
rem to find SETUP_COMPILERS_CMD or provide path here.
rem we are using vcvarsall.bat from Visual Studio.
set SETUP_COMPILERS_CMD=vcvarsall.bat


rem ---------------------------------------------------------------------------
if not [%~5]==[] set CMAKE_GENERATOR=%~5
if not [%~4]==[] set TESTS_PROJECT=%~4
if not [%~3]==[] set TARGET_TRIPLET=%~3
if not [%~2]==[] set BUILD_TYPE=%~2

echo.
echo %~0 %*
echo ---
echo BUILD_TYPE:      %BUILD_TYPE%
echo TARGET_TRIPLET:  %TARGET_TRIPLET%
echo TESTS_PROJECT:   %TESTS_PROJECT%
echo CMAKE_GENERATOR: %CMAKE_GENERATOR%
echo ---
set ARGS=%BUILD_TYPE% %TARGET_TRIPLET% %TESTS_PROJECT% %CMAKE_GENERATOR%

rem ---------------------------------------------------------------------------
rem Setup correct compiler from Visual Studio using vcvarsall.bat

rem Check if user's windows os is 32-bit or 64-bit.
if /I [%PROCESSOR_ARCHITECTURE%]==[x86] (
    if [%PROCESSOR_ARCHITEW6432%]==[] (
        set HOST_ARCH=x86
    ) else (
        set HOST_ARCH=x64
    )
)
if /I [%PROCESSOR_ARCHITECTURE%]==[AMD64] (
    set HOST_ARCH=x64
)
if [%TARGET_TRIPLET%]==[x64-windows] (set TARGET_ARCH=x64)
if [%TARGET_TRIPLET%]==[x86-windows] (set TARGET_ARCH=x86)

set ARCH_TRIPLET=%HOST_ARCH%_%TARGET_ARCH%
echo ARCH_TRIPLET: %ARCH_TRIPLET%

rem we are using vcvarsall.bat from Visual Studio.
rem Ensure that your Jenkins Windows Agent has PATH set up to find vcvarsall.bat
rem e.g. set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;^%PATH^%
rem when you want to use other compilers, provide a setup script at SETUP_COMPILERS_CMD 
rem that either accepts the following triplets, 
rem or you change the triplets generated here to match the setup script.

rem vcvarsall.bat [x86 | amd64 | x86_amd64 | x86_arm | x86_arm64 | amd64_x86 | amd64_arm | amd64_arm64]
if [%ARCH_TRIPLET%]==[x64_x64] (set SETUP_COMPILERS_TRIPLET=amd64)
if [%ARCH_TRIPLET%]==[x64_x86] (set SETUP_COMPILERS_TRIPLET=amd64_x86)
if [%ARCH_TRIPLET%]==[x86_x64] (set SETUP_COMPILERS_TRIPLET=x86_amd64)
if [%ARCH_TRIPLET%]==[x86_x86] (set SETUP_COMPILERS_TRIPLET=x86)
echo SETUP_COMPILERS_TRIPLET: %SETUP_COMPILERS_TRIPLET%
echo SETUP_COMPILERS_CMD: %SETUP_COMPILERS_CMD%
echo ---

rem ---------------------------------------------------------------------------
if [%~1]==[all]         goto all
if [%~1]==[clean]       goto clean
if [%~1]==[tools]       goto tools
if [%~1]==[build]       goto build
if [%~1]==[test]        goto test
if [%~1]==[build_test]  goto build_test
if [%~1]==[help]        goto help
if [%~1]==[-]           goto build_test
%0 build_test %*
goto exit

rem ---------------------------------------------------------------------------
:all
call %~0 clean %ARGS% && call %~0 tools %ARGS% && call %~0 build %ARGS% && call %~0 test %ARGS%
goto exit

rem ---------------------------------------------------------------------------
:build_test
call %~0 build %ARGS% && call %~0 test %ARGS%
goto exit

rem ---------------------------------------------------------------------------
:clean
echo Cleaning up...
if exist "%~dp0\tools\" (
    rmdir /Q /S "%DIR%\tools\"
)
if exist "%DIR%\build\" (
    rmdir /Q /S "%DIR%\build\"
)
goto exit

rem ---------------------------------------------------------------------------
:tools
echo Setup Compiler Tools...
call %SETUP_COMPILERS_CMD% %SETUP_COMPILERS_TRIPLET%

echo Preparing tools...
if not exist "%DIR%\tools\" (
    mkdir "%DIR%\tools\"
)
if not exist "%DIR%\tools\vcpkg\vcpkg" (
    git clone --depth 1 --single-branch --branch imgui-cpp11 https://github.com/xaedes/vcpkg.git "%DIR%\\tools\\vcpkg/"
    cd "%DIR%\\tools\\vcpkg\\"
    git pull origin imgui-cpp11
    cd "%DIR%\\"
    rem git clone https://github.com/microsoft/vcpkg.git "%DIR%\\tools\\vcpkg\\"
    "%DIR%\tools\vcpkg\bootstrap-vcpkg.bat" -disableMetrics
)
goto exit

rem ---------------------------------------------------------------------------
:build
echo Setup Compiler Tools...
call %SETUP_COMPILERS_CMD% %SETUP_COMPILERS_TRIPLET%

echo Building...
set VCPKG_FEATURE_FLAGS=versions
set VCPKG_TARGET_TRIPLET=%TARGET_TRIPLET%
if not exist "%DIR%\build\Windows\%TARGET_TRIPLET%\%BUILD_TYPE%" (
    mkdir "%DIR%\build\Windows\%TARGET_TRIPLET%\%BUILD_TYPE%"
)
echo on
cd %DIR%\build\Windows\%TARGET_TRIPLET%\%BUILD_TYPE%
cmake --version
cmake -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=%DIR%\tools\vcpkg\scripts\buildsystems\vcpkg.cmake %DIR%
cd %DIR%
cmake --build %DIR%\build\Windows\%TARGET_TRIPLET%\%BUILD_TYPE%
@echo off
goto exit

rem ---------------------------------------------------------------------------
:test
echo Setup Compiler Tools...
call %SETUP_COMPILERS_CMD% %SETUP_COMPILERS_TRIPLET%

echo Testing...
echo on
cd "%DIR%\build\Windows\%TARGET_TRIPLET%\%BUILD_TYPE%\%TESTS_PROJECT%\"
ctest --version
ctest || ctest --rerun-failed --output-on-failure
cd %DIR%
@echo off
goto exit

rem ---------------------------------------------------------------------------
:help
echo Usage:
echo %~0 [all^|clean^|tools^|build^|test^|build_test^|-^|help]

rem ---------------------------------------------------------------------------
:exit

endlocal

exit /b 0
