@echo off

del /q .qmake.stash
del /q Makefile*
if %BuildType%==release (
rmdir release /s /q
mkdir release
) else (
rmdir debug /s /q
mkdir debug
)
rmdir build /s /q
mkdir build