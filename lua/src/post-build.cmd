@ECHO OFF
SET SrcDir=%~dp0
ECHO 正在生成 LUA 控制台... 

SET Configuration=%1
SET SolutionDir=%2
SET WorkDir=%SolutionDir%Output\lua\%Configuration%
Set OutDir=%SolutionDir%%Configuration%

ECHO %SrcDir%
IF "%1" == "Debug" GOTO :BuildDebug
IF "%1" == "Release" GOTO :BuildRelease

GOTO :EXITLABEL

:BuildDebug
echo 编译调试版本...%SrcDir%
echo CL /Od /Gs /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "_DEBUG" %SrcDir%\lua.c /Fo"%WorkDir%"\lua.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\lua.exe
CL /Od /GS /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "_DEBUG" %SrcDir%\lua.c /Fo"%WorkDir%"\lua.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\lua.exe >nul
CL /Od /GS /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "_DEBUG" %SrcDir%\luac.c /Fo"%WorkDir%"\luac.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\luac.exe >nul

GOTO :EXITLABEL
:BuildRelease
echo "编译发布版本..."
echo CL /Od /Gs /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "_DEBUG" %SrcDir%\lua.c /Fo"%WorkDir%"\lua.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\lua.exe
CL /O2 /GS /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "NDEBUG" %SrcDir%\lua.c /Fo"%WorkDir%"\lua.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\lua.exe
CL /O2 /GS /GL /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /D "NDEBUG" %SrcDir%\luac.c /Fo"%WorkDir%"\lua.obj /link lua.lib /LIBPATH:%OutDir% /OUT:%OutDir%\luac.exe

GOTO :EXITLABEL

REM FOR /F "usebackq tokens=1* delims==" %%i IN (`set`) DO @echo [%%i----%%j]
:EXITLABEL