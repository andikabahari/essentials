@echo off
@setlocal

:: Default

set opt_impl=0
set opt_debug=0

:: Parse args

:parse
if "%1"=="" goto done_parse

if /I "%1"=="impl"  set opt_impl=1
if /I "%1"=="debug" set opt_debug=1

shift
goto parse

:done_parse

:: Compiler flags

set cflags=
if %opt_debug%==1 (set cflags=/Zi /Od /DDEBUG) else (set cflags=/O2)
if %opt_impl%==1  (set cflags=%cflags% /DBASE_STATIC /DBASE_COMPILED)

set lflgas=
if %opt_debug%==1 set lflags=/DEBUG

:: Compile

set cl_base=
set base_obj=
if %opt_impl%==1 set cl_base=call cl /c /TP %cflags% /DBASE_IMPLEMENTATION base.h /nologo
if %opt_impl%==1 (
    echo Compiling base implementation...
    echo Command: %cl_base%
    %cl_base%
    echo.
    set base_obj=base.obj
)

set cl_tests=call cl %cflags% tests.cpp %base_obj% %lflags% /nologo
echo Compiling tests...
echo Command: %cl_tests%
if exist "tests.exe" del tests.exe
%cl_tests%
echo.

:: Test!

if exist "tests.exe" call tests.exe
echo Done.

@endlocal
