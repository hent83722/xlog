#Requires -Version 5.1
<#
.SYNOPSIS
    Zyrnix Installation Script for Windows (PowerShell)
    
.DESCRIPTION
    Builds Zyrnix from source and installs it system-wide.
    Run as Administrator for system-wide installation.
    
.PARAMETER BuildType
    Build configuration: Release or Debug (default: Release)
    
.PARAMETER InstallPrefix
    Installation directory (default: C:\Program Files\Zyrnix)
    
.PARAMETER Generator
    CMake generator to use (e.g., "Visual Studio 17 2022")
    
.PARAMETER Jobs
    Number of parallel build jobs (default: number of CPU cores)
    
.EXAMPLE
    .\install_windows.ps1
    
.EXAMPLE
    .\install_windows.ps1 -BuildType Debug -InstallPrefix "C:\dev\Zyrnix"
    
.EXAMPLE
    .\install_windows.ps1 -Generator "Visual Studio 17 2022" -Jobs 8
#>

param(
    [ValidateSet("Release", "Debug")]
    [string]$BuildType = "Release",
    
    [string]$InstallPrefix = "C:\Program Files\Zyrnix",
    
    [string]$Generator = "",
    
    [int]$Jobs = 0
)

$ErrorActionPreference = "Stop"

# Colors
function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

# Banner
Write-Host ""
Write-Host "██╗  ██╗██╗      ██████╗  ██████╗ " -ForegroundColor Blue
Write-Host "╚██╗██╔╝██║     ██╔═══██╗██╔════╝ " -ForegroundColor Blue
Write-Host " ╚███╔╝ ██║     ██║   ██║██║      " -ForegroundColor Blue
Write-Host " ██╔██╗ ██║     ██║   ██║██║   ██║" -ForegroundColor Blue
Write-Host "██╔╝ ██╗███████╗╚██████╔╝╚██████╔╝" -ForegroundColor Blue
Write-Host "╚═╝  ╚═╝╚══════╝ ╚═════╝  ╚═════╝ " -ForegroundColor Blue
Write-Host ""
Write-Host "Zyrnix Installation Script for Windows" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""

# Get script directory and project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir

Set-Location $ProjectDir

# Auto-detect parallel jobs
if ($Jobs -eq 0) {
    $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    if ($null -eq $Jobs -or $Jobs -eq 0) { $Jobs = 4 }
}

# Check dependencies
Write-Host "Checking dependencies..." -ForegroundColor Yellow

# Check CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    Write-Host "ERROR: cmake is not installed or not in PATH." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install CMake using one of these methods:"
    Write-Host "  - Download from: https://cmake.org/download/"
    Write-Host "  - winget: winget install Kitware.CMake"
    Write-Host "  - chocolatey: choco install cmake"
    Write-Host "  - scoop: scoop install cmake"
    exit 1
}

# Check for C++ compiler
$hasCompiler = $false
$compilerName = ""

# Check for MSVC (cl.exe)
$cl = Get-Command cl -ErrorAction SilentlyContinue
if ($null -ne $cl) {
    $hasCompiler = $true
    $compilerName = "MSVC"
}

# Check for MinGW g++
if (-not $hasCompiler) {
    $gpp = Get-Command g++ -ErrorAction SilentlyContinue
    if ($null -ne $gpp) {
        $hasCompiler = $true
        $compilerName = "MinGW g++"
    }
}

# Check for Clang
if (-not $hasCompiler) {
    $clangpp = Get-Command clang++ -ErrorAction SilentlyContinue
    if ($null -ne $clangpp) {
        $hasCompiler = $true
        $compilerName = "Clang"
    }
}

if (-not $hasCompiler) {
    Write-Host "ERROR: No C++ compiler found." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install one of the following:"
    Write-Host "  - Visual Studio with 'Desktop development with C++' workload"
    Write-Host "  - MinGW-w64: https://www.mingw-w64.org/"
    Write-Host "  - LLVM/Clang: https://releases.llvm.org/"
    Write-Host ""
    Write-Host "If Visual Studio is installed, try running from:"
    Write-Host "  'Developer PowerShell for VS' or 'x64 Native Tools Command Prompt'"
    exit 1
}

Write-Host "Found $compilerName compiler" -ForegroundColor Green
Write-Host "All dependencies found!" -ForegroundColor Green
Write-Host ""

# Display configuration
Write-Host "Configuration:" -ForegroundColor Yellow
Write-Host "  Build Type:      $BuildType"
Write-Host "  Install Prefix:  $InstallPrefix"
Write-Host "  Parallel Jobs:   $Jobs"
if ($Generator -ne "") {
    Write-Host "  Generator:       $Generator"
}
Write-Host ""

# Create build directory
Write-Host "Creating build directory..." -ForegroundColor Yellow
$BuildDir = Join-Path $ProjectDir "build"
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
Set-Location $BuildDir

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Yellow
$cmakeArgs = @(
    "..",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
)

if ($Generator -ne "") {
    $cmakeArgs += "-G"
    $cmakeArgs += $Generator
}

& cmake $cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed." -ForegroundColor Red
    exit 1
}

# Build
Write-Host "Building Zyrnix..." -ForegroundColor Yellow
& cmake --build . --config $BuildType --parallel $Jobs
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build complete!" -ForegroundColor Green
Write-Host ""

# Check for admin rights
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "WARNING: Not running as Administrator." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To install system-wide (to $InstallPrefix),"
    Write-Host "please run this script as Administrator."
    Write-Host ""
    $response = Read-Host "Continue with installation anyway? (y/N)"
    if ($response -ne "y" -and $response -ne "Y") {
        Write-Host ""
        Write-Host "Build completed but installation skipped." -ForegroundColor Yellow
        Write-Host "You can manually install later with:"
        Write-Host "  cmake --install build --config $BuildType" -ForegroundColor Cyan
        exit 0
    }
}

# Install
Write-Host "Installing Zyrnix..." -ForegroundColor Yellow
& cmake --install . --config $BuildType
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Installation failed." -ForegroundColor Red
    Write-Host "Try running as Administrator or use a different -InstallPrefix."
    exit 1
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "Zyrnix has been installed successfully!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Installation location: $InstallPrefix"
Write-Host ""
Write-Host "To use Zyrnix in your CMake project, add:"
Write-Host '  set(CMAKE_PREFIX_PATH "' -NoNewline
Write-Host $InstallPrefix -NoNewline -ForegroundColor Cyan
Write-Host '")'
Write-Host "  find_package(Zyrnix REQUIRED)" -ForegroundColor Cyan
Write-Host "  target_link_libraries(your_target PRIVATE Zyrnix::Zyrnix)" -ForegroundColor Cyan
Write-Host ""
