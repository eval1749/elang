@if "%_echo%"=="" echo off
setlocal
set outdir=..\out\Debug
echo.
echo.
echo //////////////////////////////////////////////////////////////////////
echo //
echo // Current directory is %CD%
echo // Generating *.ninja into %outdir%
echo //
cd src
call gn gen %outdir% --args="is_component_build=true"
endlocal
