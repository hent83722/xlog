@echo off
REM ============================================
REM Zyrnix Installation Script for Windows (Batch)
REM Builds from source and installs system-wide
REM Run as Administrator for system installation
REM ============================================

setlocal EnableDelayedExpansion

echo.
echo ██╗  ██╗██╗      ██████╗  ██████╗ 
echo ╚██╗██╔╝██║     ██╔═══██╗██╔════╝ 
echo  ╚███╔╝ ██║     ██║   ██║██║      
echo  ██╔██╗ ██║     ██║   ██║██║   ██║
echo ██╔╝ ██╗███████╗╚██████╔╝╚██████╔╝
echo ╚═╝  ╚═╝╚══════╝ ╚═════╝  ╚═════╝ 
echo.
echo Zyrnix Installation Script for Windows
echo ==========================================
echo.

REM Get the directory where this script is located
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."

cd /d "%PROJECT_DIR%"

REM Default settings
set "BUILD_TYPE=Release"
set "INSTALL_PREFIX=C:\Program Files\Zyrnix"
set "GENERATOR="

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :check_deps
if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="--prefix" (
    set "INSTALL_PREFIX=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--generator" (
    set "GENERATOR=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    echo Usage: %~nx0 [options]
    echo Options:
    echo   --debug              Build in debug mode ^(default: Release^)
    echo   --prefix PATH        Installation prefix ^(default: C:\Program Files\Zyrnix^)
    echo   --generator NAME     CMake generator ^(e.g., "Visual Studio 17 2022"^)
    echo   --help               Show this help message
    echo.
    echo NOTE: Run as Administrator for system-wide installation.
    exit /b 0
)
echo Unknown option: %~1
exit /b 1

:check_deps
echo Checking dependencies...

REM Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: cmake is not installed or not in PATH.
    echo.
    echo Please install CMake from: https://cmake.org/download/
    echo Or install via winget: winget install Kitware.CMake
    echo Or install via chocolatey: choco install cmake
    exit /b 1
)

REM Check for a C++ compiler (MSVC, MinGW, or Clang)
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo Found MSVC compiler
    goto :deps_ok
)

where g++ >nul 2>&1
if %errorlevel% equ 0 (
    echo Found MinGW g++ compiler
    goto :deps_ok
)

where clang++ >nul 2>&1
if %errorlevel% equ 0 (
    echo Found Clang compiler
    goto :deps_ok
)

echo ERROR: No C++ compiler found.
echo.
echo Please install one of the following:
echo   - Visual Studio with C++ workload
echo   - MinGW-w64: https://www.mingw-w64.org/
echo   - LLVM/Clang: https://releases.llvm.org/
exit /b 1

:deps_ok
echo All dependencies found!
echo.
echo Configuration:
echo   Build Type:      %BUILD_TYPE%
echo   Install Prefix:  %INSTALL_PREFIX%
if defined GENERATOR echo   Generator:       %GENERATOR%
echo.

REM Create build directory
echo Creating build directory...
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
if defined GENERATOR (
    cmake .. -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%"
) else (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%"
)

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

REM Build
echo Building Zyrnix...
cmake --build . --config %BUILD_TYPE% --parallel

if %errorlevel% neq 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo Build complete!
echo.

REM Check for admin rights before installing
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Not running as Administrator.
    echo.
    echo To install system-wide, please run this script as Administrator.
    echo Alternatively, use --prefix to specify a user-writable directory.
    echo.
    set /p "CONTINUE=Continue with installation anyway? (y/N): "
    if /i not "!CONTINUE!"=="y" (
        echo.
        echo Build completed but installation skipped.
        echo You can manually install later with:
        echo   cmake --install build --config %BUILD_TYPE%
        exit /b 0
    )
)

REM Install
echo Installing Zyrnix...
cmake --install . --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo ERROR: Installation failed.
    echo Try running as Administrator or use a different --prefix.
    exit /b 1
)

echo.
echo ==========================================
echo Zyrnix has been installed successfully!
echo ==========================================
echo.
echo Installation location: %INSTALL_PREFIX%
echo.
echo To use Zyrnix in your CMake project, add:
echo   set(CMAKE_PREFIX_PATH "%INSTALL_PREFIX%")
echo   find_package(Zyrnix REQUIRED)
echo   target_link_libraries(your_target PRIVATE Zyrnix::Zyrnix)
echo.

endlocal
