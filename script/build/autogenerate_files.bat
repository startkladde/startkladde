@echo off

REM This batch file is intended to be executed from the Windows GUI. It calls
REM the actual Ruby script and keeps the command window open so the user can
REM read the output.
REM This batch file looks for the Ruby script in the current directory. This
REM will only work when it is called from its own directory, as is the case
REM when invoked from the GUI. From the command line, call the Ruby script
REM directly.

ruby autogenerate_files.rb
pause

