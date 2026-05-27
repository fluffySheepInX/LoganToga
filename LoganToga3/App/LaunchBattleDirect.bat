@echo off
setlocal
set "ARG=%~1"
if "%ARG%"=="" set "ARG=Skirmish"
start "" "LoganToga3.exe" "--quick-battle" "%ARG%"
