@echo off

set BuildType=release

if "%1" == "" goto build

for %%x in (%*) do (
    if "%%x" == "-d" (
		set BuildType=debug
		echo Debug build
	)
)

for %%x in (%*) do (
    if "%%x" == "-c" goto clean
)
goto build

:clean
echo Cleaning...
call clean
set Cleaned=1


:build
echo Building...



qmake CONFIG+=%BuildType% DaeSolver.pro
IF %ERRORLEVEL% NEQ 0 goto failed

mingw32-make
IF %ERRORLEVEL% NEQ 0 goto failed

IF "%Cleaned%"=="1" goto deploy
goto end

:deploy
call deploy

goto end

:failed
pause

:end
set Cleaned=