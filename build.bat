@echo off
REM I acknowledge that this is not good :)
REM I just dont care rn.

setlocal enabledelayedexpansion

set DIRS=src\*.c src\osag\*.c src\vulkan\*.c src\win32\*.c
set INCLUDE=-IC:\VulkanSDK\1.4.313.0\Include
set FLAGS=-std=c99 -Wall -Wextra -O2

if not exist build\int mkdir build\int
del /Q build\int\*.o 2>nul
del build\zenlib-core.a

for %%f in (%DIRS%) do (
    echo [ZENLIB] Compiling %%f...
    gcc -c %%f %INCLUDE% %FLAGS% -o %%~nf.o
    move /Y %%~nf.o build\int\
)

echo [ZENLIB] Linking...
set OBJFILES=
for %%o in (build\int\*.o) do (
    set OBJFILES=!OBJFILES! %%o
)
ar rcs build\zenlib-core.a !OBJFILES!
echo [ZENLIB] Finished

call .\examples\vulkan\build.bat

endlocal