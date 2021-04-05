@echo off

echo This batch file, goes through the fully automated process of building Oodle dictionaries from packet captures.
echo.


REM This batch file can only work from within the Oodle folder.
REM assume you are in \Engine\Plugins\OodleNetwork
set BaseFolder="..\..\.."

if exist %BaseFolder:"=%\Engine goto SetUE4Editor

echo Could not locate Engine folder. This .bat must be located within UE4\Engine\Plugins\Runtime\PacketHandlers\CompressionComponents\Oodle
goto End


:SetUE4Editor
set UE4EditorLoc="%BaseFolder:"=%\Engine\Binaries\Win64\UE4Editor.exe"

if exist %UE4EditorLoc:"=% goto GetGame

echo Could not locate UE4Editor.exe
goto End


:GetGame
set /p GameName=Type the name of the game you are working with: 
echo.



:AutoGenDictionaries
set AutoGenDictionariesParms=-run=OodleTrainerCommandlet AutoGenerateDictionaries
set FinalGenCmdLine=%GameName:"=% %AutoGenDictionariesParms% -forcelogflush -OodleDebugDump

echo Executing dictionary generation commandlet - commandline:
echo %FinalGenCmdLine%

@echo on
%UE4EditorLoc:"=% %FinalGenCmdLine%
@echo off
echo.


if %errorlevel%==0 goto End

echo WARNING! Detected error, dictionaries may not have been generated. Check output and logfile for errors.
pause


:End
echo Execution complete.
pause


REM Put nothing past here.

