@echo off
echo "Startkladde debugging batch file"
echo.
echo.
echo.
del startkladde.log
.\startkladde_trunk.exe --no-full-screen -q >startkladde.log
type startkladde.log

