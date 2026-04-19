@echo off
@setlocal

:: Default

set opt_static=0
set opt_debug=0

:: Parse args

:parse
if "%1"=="" goto done_parse

if /I "%1"=="static" set opt_static=1
if /I "%1"=="debug" set opt_debug=1

shift
goto parse

:done_parse

:: Compiler flags

set cflags=
if %opt_debug%==1 (set cflags=/Zi /Od /DDEBUG) else (set cflags=/O2)
if %opt_static%==1 (set cflags=%cflags% /DBASE_STATIC)

set lflgas=
if %opt_debug%==1 set lflags=/DEBUG

:: Compile

set dir_includes=/I SDL\include
set dir_libs=/LIBPATH:SDL\lib\x64

set cl_base=
set base_obj=
if %opt_static%==1 set cl_base=call cl /c /TP %cflags% /DBASE_IMPLEMENTATION base.h /nologo
if %opt_static%==1 (
    echo Compiling base.h...
    echo Command: %cl_base%
    %cl_base%
    echo.
    set base_obj=base.obj
)

set cl_gfx=
set gfx_obj=
if %opt_static%==1 set cl_gfx=call cl /c /TP %cflags% /DGFX_IMPLEMENTATION %dir_includes% gfx.h /nologo
if %opt_static%==1 (
    echo Compiling gfx.h...
    echo Command: %cl_gfx%
    %cl_gfx%
    echo.
    set gfx_obj=gfx.obj
)

set obj_flags=
if not "%base_obj%"==""     set obj_flags=%obj_flags% /DBASE_COMPILED
if not "%gfx_obj%"=="" set obj_flags=%obj_flags% /DGFX_COMPILED

set libs=%base_obj% %gfx_obj% SDL3.lib

set cl_test=call cl %cflags% %obj_flags% %dir_includes% test\test_all.cpp /link %lflags% %dir_libs% %libs% /nologo
echo Compiling tests...
echo Command: %cl_test%
if exist "test_all.exe" del test_all.exe
%cl_test%
set test_exe=test_all.exe
echo.

:: Test!

copy SDL\lib\x64\SDL3.dll SDL3.dll >nul 2>&1
if exist %test_exe% call %test_exe%

echo Done.

@endlocal
