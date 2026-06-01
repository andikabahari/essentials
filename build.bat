@echo off
@setlocal

:: Default

set opt_debug=0

:: Parse args

:parse
if "%1"=="" goto done_parse

if /I "%1"=="debug" set opt_debug=1

shift
goto parse

:done_parse

:: Compiler flags

set cflags=
if %opt_debug%==1 (set cflags=/Zi /Od /D"BUILD_DEBUG=1") else (set cflags=/O2 /D"BUILD_DEBUG=0")

set lflgas=
if %opt_debug%==1 set lflags=/DEBUG

:: Config vars

set include_dirs=/I SDL\include
set lib_dirs=/LIBPATH:SDL\lib\x64
set lib_files=SDL3.lib

copy SDL\lib\x64\SDL3.dll SDL3.dll >nul 2>&1

:: Tests and samples

if exist test_base.exe       del test_base.exe
if exist test_linalg.exe     del test_linalg.exe
if exist sample_triangle.exe del sample_triangle.exe

set cl_test_base=call cl /Zc:__cplusplus %cflags% test\test_base.cpp base.cpp /nologo
echo Command: %cl_test_base%
%cl_test_base%
if errorlevel 1 exit /b 1

set cl_test_linalg=call cl /Zc:__cplusplus %cflags% test\test_linalg.cpp linalg.cpp /nologo
echo Command: %cl_test_linalg%
%cl_test_linalg%
if errorlevel 1 exit /b 1

set cl_sample_triangle=call cl /Zc:__cplusplus %cflags% %include_dirs% test\sample_triangle.cpp base.cpp linalg.cpp gfx.cpp /link %lflags% %lib_dirs% %lib_files% /nologo
echo Command: %cl_sample_triangle%
%cl_sample_triangle%
if errorlevel 1 exit /b 1

call test_base.exe
call test_linalg.exe
call sample_triangle.exe

@endlocal
