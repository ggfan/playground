#!/bin/bash
SDK_SRC=$HOME/Proj/games/games-sdk-src/gamesdk
APP_SRC=$HOME/Proj/playground/android/games/gamesdk/GameActivity

# build the sdk
pushd $SDK_SRC
./gradlew packageLocalZip -Plibraries=game_activity

# pick up the game-activity to the app
cp -fr $SDK_SRC/../package/gamesdk/GameActivity-release.aar  $APP_SRC/app/libs

# force re-sync in the IDE
popd
rm -fr app/build  app/.cxx

