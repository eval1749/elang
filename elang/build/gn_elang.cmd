@if "%_echo%"=="" echo off
setlocal
call gn gen out\Debug --args="is_component_build=true"
endlocal
