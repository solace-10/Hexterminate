#!/bin/bash

if [ $# != 1 ] || ( [ $1 != "standalone" ] && [ $1 != "steam" ] ); then
    echo Expected one argument: "standalone" or "steam".
    exit 1
fi

echo === Started publishing ===
echo Target: $1
cd ../../..
PROJECT_ROOT=$PWD
HEXTERMINATE_DIR=$PROJECT_ROOT/Game/bin
INTERMEDIATES_DIR=$PROJECT_ROOT/Game/tools/publish/intermediates
INTERMEDIATES_GAME_DIR=$INTERMEDIATES_DIR/game
INTERMEDIATES_BACKTRACE_DIR=$INTERMEDIATES_DIR/backtrace

echo === Getting Hexterminate\'s version ===
VERSION=$(<$HEXTERMINATE_DIR/version.txt)
echo Version: $VERSION

echo === Building makefiles ===

mkdir build 2>/dev/null
cd build

if [ $1 == "steam" ]; then
    USE_STEAM=ON
else
    USE_STEAM=OFF
fi

cmake -G "Unix Makefiles" -DUSE_STEAM=$USE_STEAM -DHEXTERMINATE_BUILD_VERSION=\"$VERSION\" -DCMAKE_BUILD_TYPE=Release ..

if [ $? != 0 ]; then
    echo Makefile generation failed.
    exit $?
fi

echo === Compiling ===

make -j8

if [ $? != 0 ]; then
    echo Compilation failed.
    exit $?
fi

echo === Copying to intermediates ===

INTERMEDIATES_DIR=$PROJECT_ROOT/Game/tools/publish/intermediates
echo Intermediates: $INTERMEDIATES_DIR
rm -rf $INTERMEDIATES_DIR 2>/dev/null
mkdir $INTERMEDIATES_DIR
mkdir $INTERMEDIATES_BACKTRACE_DIR
mkdir $INTERMEDIATES_GAME_DIR
mkdir $INTERMEDIATES_GAME_DIR/crashhandler

cp $HEXTERMINATE_DIR/Hexterminate $INTERMEDIATES_GAME_DIR
cp $HEXTERMINATE_DIR/crashhandler/crashpad_handler $INTERMEDIATES_GAME_DIR/crashhandler/crashpad_handler
cp -r $HEXTERMINATE_DIR/data $INTERMEDIATES_GAME_DIR/data
if [ $1 == "steam" ]; then
    cp $PROJECT_ROOT/Genesis/libs/steamworks/sdk/redistributable_bin/linux64/libsteam_api.so $INTERMEDIATES_GAME_DIR
fi

OUTPUT_DIR=$PROJECT_ROOT/Game/tools/publish/output
mkdir $OUTPUT_DIR 2>/dev/null

echo === Preparing symbols ===

echo Copying symbols...
objcopy --only-keep-debug $INTERMEDIATES_GAME_DIR/Hexterminate $INTERMEDIATES_BACKTRACE_DIR/Hexterminate.sym

echo Stripping executable...
strip --strip-debug --strip-unneeded $INTERMEDIATES_GAME_DIR/Hexterminate

echo Copying stripped executable...
cp $INTERMEDIATES_GAME_DIR/Hexterminate $INTERMEDIATES_BACKTRACE_DIR/Hexterminate

if [ $1 == "standalone" ]; then
    echo === Creating archive ===

    HEXTERMINATE_ARCHIVE=$OUTPUT_DIR/Hexterminate-linux64-$VERSION-standalone.zip
    rm $HEXTERMINATE_ARCHIVE 2>/dev/null

    HEXTERMINATE_SYMBOLS_ARCHIVE=$OUTPUT_DIR/Hexterminate-linux64-$VERSION-standalone-symbols.zip
    rm $HEXTERMINATE_SYMBOLS_ARCHIVE 2>/dev/null

    cd $INTERMEDIATES_GAME_DIR
    zip -r $HEXTERMINATE_ARCHIVE .

    if [ $? != 0 ]; then
        echo Archive creation failed.
        exit $?
    fi

    echo Archive created in \'$HEXTERMINATE_ARCHIVE\'.
elif [ $1 == "steam" ]; then

    echo === Creating archive ===

    HEXTERMINATE_ARCHIVE=$OUTPUT_DIR/Hexterminate-linux64-$VERSION-steam.zip
    rm $HEXTERMINATE_ARCHIVE 2>/dev/null

    HEXTERMINATE_SYMBOLS_ARCHIVE=$OUTPUT_DIR/Hexterminate-linux64-$VERSION-steam-symbols.zip
    rm $HEXTERMINATE_SYMBOLS_ARCHIVE 2>/dev/null

    cd $INTERMEDIATES_GAME_DIR
    zip -r $HEXTERMINATE_ARCHIVE .

    if [ $? != 0 ]; then
        echo Archive creation failed.
        exit $?
    fi
    echo Archive created in \'$HEXTERMINATE_ARCHIVE\'. 

    read -p "Run SteamPipe? (Y/N): " RUN_STEAM_PIPE

    if [[ $RUN_STEAM_PIPE == [yY] ]]; then
        echo === Running SteamPipe ===

        if [[ -z "${HEXTERMINATE_STEAM_USERNAME}" ]]; then
            echo Environment variable HEXTERMINATE_STEAM_USERNAME must be set.
            exit -1
        fi

        if [[ -z "${HEXTERMINATE_STEAM_PASSWORD}" ]]; then
            echo Environment variable HEXTERMINATE_STEAM_PASSWORD must be set.
            exit -1
        fi

        echo SteamPipe depot:
        echo 1. Experimental
        echo 2. Main

        read DEPOT
        STEAMCMD=$PROJECT_ROOT/Genesis/libs/steamworks/sdk/tools/ContentBuilder/builder_linux/steamcmd.sh
        if [ $DEPOT == "1" ]; then
            $STEAMCMD +login $HEXTERMINATE_STEAM_USERNAME $HEXTERMINATE_STEAM_PASSWORD +run_app_build $PROJECT_ROOT/Game/tools/publish/steampipe/linux_experimental.vdf +quit
        elif [ $DEPOT == "2" ]; then
            $STEAMCMD +login $HEXTERMINATE_STEAM_USERNAME $HEXTERMINATE_STEAM_PASSWORD +run_app_build $PROJECT_ROOT/Game/tools/publish/steampipe/linux_main.vdf +quit
        else
            echo Invalid option.
            exit -1
        fi

        if [ $? != 0 ]; then
            echo SteamPipe failed.
            exit $?
        fi
    fi
fi

echo === Archiving symbols ===

if [[ -z "${HEXTERMINATE_BACKTRACE_TOKEN}" ]]; then
    echo Environment variable HEXTERMINATE_BACKTRACE_TOKEN must be set.
    exit -1
fi

cd $INTERMEDIATES_BACKTRACE_DIR

# For Backtrace we need to upload both the binary and the symbols. We put them in a single archive and send it to the server.
BACKTRACE_SYMBOLS_ARCHIVE=$INTERMEDIATES_BACKTRACE_DIR/symbols.zip
echo Creating zip file...
rm $BACKTRACE_SYMBOLS_ARCHIVE 2> /dev/null
zip -j $BACKTRACE_SYMBOLS_ARCHIVE Hexterminate Hexterminate.sym

echo Uploading to backtrace.io...
curl --data-binary @$BACKTRACE_SYMBOLS_ARCHIVE -X POST -H "Expect: gzip" "https://submit.backtrace.io/solace10/$HEXTERMINATE_BACKTRACE_TOKEN/symbols"

# We also keep a local copy of the symbols, as we can't download them from Backtrace.
cp symbols.zip $HEXTERMINATE_SYMBOLS_ARCHIVE

if [ $? != 0 ]; then
    echo Failed to upload symbols.
    exit $?
fi