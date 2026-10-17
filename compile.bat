@echo off

where clang >nul 2>nul
if not errorlevel 1 goto clang

where cl >nul 2>nul
if not errorlevel 1 goto msvc

echo compile.bat: no C compiler found (install LLVM clang or use a Developer Command Prompt) 1>&2
exit /b 1

:clang
clang -std=c99 -Wall -Wextra -O2 -D_CRT_SECURE_NO_WARNINGS src\bcbld.c -o bcbld.exe
exit /b %errorlevel%

:msvc
cl /nologo /std:c11 /W3 /O2 /D_CRT_SECURE_NO_WARNINGS src\bcbld.c /Fe:bcbld.exe /Fo:bcbld.obj
set result=%errorlevel%
del /q bcbld.obj 2>nul
exit /b %result%
