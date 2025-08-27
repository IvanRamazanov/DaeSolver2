@echo off
SetLocal EnableDelayedExpansion
(set PATH=C:\dev\qt-src-6.9.0\qtbase\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=C:\dev\qt-src-6.9.0\qtbase\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=C:\dev\qt-src-6.9.0\qtbase\plugins
)
%*
EndLocal
