if "%BuildType%"=="debug" (
    windeployqt ./debug/DaeSolver.exe
) else (
    windeployqt ./release/DaeSolver.exe
)