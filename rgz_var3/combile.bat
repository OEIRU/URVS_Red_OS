@echo off
echo Cleaning project...
if exist *.exe del /f /q *.exe
if exist *.dll del /f /q *.dll
if exist *.obj del /f /q *.obj
if exist *.lib del /f /q *.lib
if exist *.exp del /f /q *.exp

echo Compiling main module (32-bit)...
cl /Fe:RGZ_URVS.exe main.cpp kernel32.lib user32.lib gdi32.lib advapi32.lib
if errorlevel 1 (
    echo Error: Compilation of main module failed!
    pause
    exit /b 1
)

echo Compiling dynamic library (32-bit)...
cl /LD /Fe:winlib.dll winlib.cpp kernel32.lib user32.lib gdi32.lib advapi32.lib
if errorlevel 1 (
    echo Error: Compilation of dynamic library failed!
    pause
    exit /b 1
)

echo Cleaning up...
del /f /q *.obj *.lib *.exp

echo Build complete!
pause