@echo off
set SOURCE=src\main.c src\c\*.c src\c\win32\*.c
set FLAGS=-std=c99 -Wall -Wextra -luser32 -lkernel32

@echo on
gcc %FLAGS% %SOURCE% -o build\engine.exe
build\engine.exe