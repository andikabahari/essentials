@echo off

:: Default

set opt_impl=0
set opt_debug=0

:: Parse args

:parse
if "%1"=="" goto done_parse

if /I "%1"=="impl" (
    set opt_impl=1
)

if /I "%1"=="debug" (
    set opt_debug=1
)

shift
goto parse

:done_parse

:: Compiler flags

if %opt_debug%==1 (
    set cflags=%cflags% /Zi /Od /DDEBUG
    set lflags=/DEBUG
) else (
    set cflags=%cflags% /O2
    set lflags=
)

:: Build base

if %opt_impl%==1 (
    echo Compiling base implementation...
    cl /c %cflags% base.h /DBASE_IMPLEMENTATION
    set base_obj=base.obj
) else (
    set base_obj=
)

:: Build examples

echo Building examples...
cl %cflags% example.cpp example2.cpp %base_obj% %lflags%

echo Done.
