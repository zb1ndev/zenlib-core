@echo off
setlocal enabledelayedexpansion

set INCLUDE=-IC:\VulkanSDK\1.4.313.0\Include
set LIBS=-LC:\VulkanSDK\1.4.313.0\Lib -lvulkan-1 -luser32 -lkernel32 -lgdi32
set FLAGS=-std=c99 -Wall -Wextra

if not exist build\int mkdir build\int
del /Q build\int\*.o 2>nul

for %%f in (src\c\*.c src\c\win32\*.c) do (
    echo Compiling %%f...
    gcc -c %%f %INCLUDE% %FLAGS% -o %%~nf.o
    move /Y %%~nf.o build\int\
)

echo Linking...
set OBJFILES=
for %%o in (build\int\*.o) do (
    set OBJFILES=!OBJFILES! %%o
)
ar rcs build\zenlib-core.a !OBJFILES!

endlocal