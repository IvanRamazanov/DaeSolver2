@echo off
SET QTEST_FUNCTION_TIMEOUT=10000

qmake tests/test_symbolic_algebra/test_symbolic_algebra.pro
IF %ERRORLEVEL% NEQ 0 goto end

mingw32-make
IF %ERRORLEVEL% NEQ 0 goto end

tests\build\test_symbolic_algebra.exe

:end
