@echo off
setlocal enabledelayedexpansion

if "%1"=="standalone" goto valid_arguments
if "%1"=="steam" goto valid_arguments

echo Expected one argument: "standalone" or "steam".
exit /b 1

:valid_arguments

echo === Started publishing ===
echo Target: %1
cd ..\..\..
set "PROJECT_ROOT=%cd%"
set "HEXTERMINATE_DIR=%PROJECT_ROOT%\Game\bin"
set "INTERMEDIATES_DIR=%PROJECT_ROOT%\Game\tools\publish\intermediates"
set "INTERMEDIATES_GAME_DIR=%INTERMEDIATES_DIR%\game"
set "INTERMEDIATES_BACKTRACE_DIR=%INTERMEDIATES_DIR%\backtrace"

echo === Getting Hexterminate's version ===
set /p VERSION=< %HEXTERMINATE_DIR%\version.txt
echo Version: %VERSION%

echo === Building Visual Studio solution ===

mkdir build 2>NUL
cd build

set "USE_STEAM=OFF"
if "%1"=="steam" set "USE_STEAM=ON"

cmake -G "Visual Studio 17 2022" -DUSE_STEAM=%USE_STEAM% -DHEXTERMINATE_BUILD_VERSION=\"%VERSION%\" -DCMAKE_BUILD_TYPE=Release ..

if %ERRORLEVEL% NEQ 0 (
    echo Project generation failed.
    cd %~dp0
    exit /b %ERRORLEVEL%
)

echo === Compiling ===

"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe" Hexterminate.sln /property:Configuration=Release

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    cd %~dp0
    exit /b %ERRORLEVEL%
)

cd %~dp0

echo === Copying to intermediates ===

if exist %INTERMEDIATES_DIR% rmdir %INTERMEDIATES_DIR% /q /s
mkdir %INTERMEDIATES_DIR%
mkdir %INTERMEDIATES_BACKTRACE_DIR%
mkdir %INTERMEDIATES_GAME_DIR%
mkdir %INTERMEDIATES_GAME_DIR%/crashhandler

copy %HEXTERMINATE_DIR%\Hexterminate.exe %INTERMEDIATES_GAME_DIR%
copy %HEXTERMINATE_DIR%\crashhandler\crashpad_handler.exe %INTERMEDIATES_GAME_DIR%\crashhandler\crashpad_handler.exe
xcopy %HEXTERMINATE_DIR%\data %INTERMEDIATES_GAME_DIR%\data /s /i
if "%1"=="steam" copy %HEXTERMINATE_DIR%\steam_api64.dll %INTERMEDIATES_GAME_DIR%

set "OUTPUT_DIR=%PROJECT_ROOT%\Game\tools\publish\output"
mkdir %OUTPUT_DIR% 2>NUL

if "%1"=="standalone" (
    echo === Creating archive ===

    set "HEXTERMINATE_ARCHIVE=%OUTPUT_DIR%\Hexterminate-win64-%VERSION%-standalone.zip"
    del !HEXTERMINATE_ARCHIVE! 2>NUL

    set "HEXTERMINATE_SYMBOLS_ARCHIVE=%OUTPUT_DIR%\Hexterminate-win64-%VERSION%-standalone-symbols.zip"
    del !HEXTERMINATE_SYMBOLS_ARCHIVE! 2>NUL

    powershell.exe -nologo -noprofile -command "& { Compress-Archive -Path %INTERMEDIATES_GAME_DIR% -DestinationPath !HEXTERMINATE_ARCHIVE! }"
    echo Archive created in '!HEXTERMINATE_ARCHIVE!'. 
)

if "%1"=="steam" (
    echo === Creating archive ===

    set "HEXTERMINATE_ARCHIVE=%OUTPUT_DIR%\Hexterminate-win64-%VERSION%-steam.zip"
    del !HEXTERMINATE_ARCHIVE! 2>NUL

    set "HEXTERMINATE_SYMBOLS_ARCHIVE=%OUTPUT_DIR%\Hexterminate-win64-%VERSION%-steam-symbols.zip"
    del !HEXTERMINATE_SYMBOLS_ARCHIVE! 2>NUL

    powershell.exe -nologo -noprofile -command "& { Compress-Archive -Path %INTERMEDIATES_GAME_DIR% -DestinationPath !HEXTERMINATE_ARCHIVE! }"
    echo Archive created in '!HEXTERMINATE_ARCHIVE!'. 

    choice /C:YN /M "Run SteamPipe?"
    if "%ERRORLEVEL%"=="1" ( 
        echo === Running SteamPipe ===

        if not defined HEXTERMINATE_STEAM_USERNAME (
            echo Environment variable HEXTERMINATE_STEAM_USERNAME must be set.
            cd ~%dp0
            exit /b %ERRORLEVEL%
        )

        if not defined HEXTERMINATE_STEAM_PASSWORD (
            echo Environment variable HEXTERMINATE_STEAM_PASSWORD must be set.
            cd ~%dp0
            exit /b %ERRORLEVEL%
        )

        echo SteamPipe depot:
        echo 1. Experimental
        echo 2. Main
        choice /C:12
        if "%ERRORLEVEL%"=="1" (
            %PROJECT_ROOT%\Genesis\libs\steamworks\sdk\tools\ContentBuilder\builder\steamcmd.exe +login %HEXTERMINATE_STEAM_USERNAME% %HEXTERMINATE_STEAM_PASSWORD% +run_app_build %~dp0\steampipe\windows_experimental.vdf +quit
        )

        if "%ERRORLEVEL%"=="2" (
            %PROJECT_ROOT%\Genesis\libs\steamworks\sdk\tools\ContentBuilder\builder\steamcmd.exe +login %HEXTERMINATE_STEAM_USERNAME% %HEXTERMINATE_STEAM_PASSWORD% +run_app_build %~dp0\steampipe\windows_main.vdf +quit
        )
    )
)

echo === Archiving symbols ===

if not defined HEXTERMINATE_BACKTRACE_TOKEN (
    echo Environment variable HEXTERMINATE_BACKTRACE_TOKEN must be set.
    exit /b -1
)

rem For Backtrace we need to upload both the binary and the PDBs. We put them in a single archive and send it to the server.
set "BACKTRACE_SYMBOLS_ARCHIVE=%INTERMEDIATES_BACKTRACE_DIR%\symbols.zip"
del %BACKTRACE_SYMBOLS_ARCHIVE% 2>NUL
powershell.exe -nologo -noprofile -command "& { Compress-Archive -Path %HEXTERMINATE_DIR%\Hexterminate.exe,%HEXTERMINATE_DIR%\Hexterminate.pdb -DestinationPath %BACKTRACE_SYMBOLS_ARCHIVE% }"

rem echo Uploading to backtrace.io...
curl --data-binary @%BACKTRACE_SYMBOLS_ARCHIVE% -X POST -H "Expect: gzip" "https://submit.backtrace.io/solace10/%HEXTERMINATE_BACKTRACE_TOKEN%/symbols"

rem We also keep a local copy of the symbols, as we can't download them from backtrace.
powershell.exe -nologo -noprofile -command "& { Compress-Archive -Path %HEXTERMINATE_DIR%\Hexterminate.pdb -DestinationPath !HEXTERMINATE_SYMBOLS_ARCHIVE! }"
echo Symbols archive created in '!HEXTERMINATE_SYMBOLS_ARCHIVE!'. 

cd %~dp0