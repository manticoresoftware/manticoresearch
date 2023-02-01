#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ -z "$CI_COMMIT_SHA" ]; then
    echo -e "${RED}CI_COMMIT_SHA not passed. Exiting$NC \n"
    exit 1
fi

CI_COMMIT_SHORT_SHA=${CI_COMMIT_SHA:0:9}

echo $CI_COMMIT_SHORT_SHA

if [[ -z "$DESTINATION" || $DESTINATION != "release" ]]; then
    DESTINATION="dev"
fi

WIN_REPO="https://repo.manticoresearch.com/repository/manticoresearch_windows/$DESTINATION/x64/"


REPO_CONTENT=$(curl $WIN_REPO 2>/dev/null)
DEPS_PATH="deps.txt"
BUDDY_COMMIT_SHA=$(cat $DEPS_PATH | grep buddy | cut -d" " -f4)
EXECUTOR_COMMIT_SHA=$(cat $DEPS_PATH | grep executor | cut -d" " -f4)
MCL_COMMIT_SHA=$(cat $DEPS_PATH | grep mcl | cut -d" " -f4)


BUDDY_PACKAGE_NAME=$(echo "$REPO_CONTENT" | grep $BUDDY_COMMIT_SHA | grep buddy | tail -n 1 | cut -d'"' -f2)
EXECUTOR_PACKAGE_NAME=$(echo "$REPO_CONTENT" | grep $EXECUTOR_COMMIT_SHA | grep executor | tail -n 1 | cut -d'"' -f2)
MCL_PACKAGE_NAME=$(echo "$REPO_CONTENT" | grep $MCL_COMMIT_SHA | grep columnar | grep libs.zip | tail -n 1 | cut -d'"' -f2)
MANTICORE_PACKAGE_NAME=$(echo "$REPO_CONTENT" | grep $CI_COMMIT_SHORT_SHA | grep main | tail -n 1 | cut -d'"' -f2)

if [ -z "$MANTICORE_PACKAGE_NAME" ] || [ -z "$BUDDY_PACKAGE_NAME" ] || [ -z "$EXECUTOR_PACKAGE_NAME" ] || [ -z "$MCL_PACKAGE_NAME" ]; then

  [ -z "$MANTICORE_PACKAGE_NAME" ] && echo -e "${RED}Can't parse Manticore package name. Exiting$NC \n"
  [ -z "$BUDDY_PACKAGE_NAME" ] && echo -e "${RED}Can't parse Buddy package name. Exiting$NC \n"
  [ -z "$EXECUTOR_PACKAGE_NAME" ] && echo -e "${RED}Can't parse Executor package name. Exiting$NC \n"
  [ -z "$MCL_PACKAGE_NAME" ] && echo -e "${RED}Can't parse Columnar package name. Exiting$NC \n"

  exit 1

fi

echo -e "${GREEN}Save src to temp files$NC"

NSIS_BUILD_DIR="/tmp/nsis_build"

echo -n "$WIN_REPO$MANTICORE_PACKAGE_NAME" > "${NSIS_BUILD_DIR}/manticore_src.txt"
echo -n "$WIN_REPO$BUDDY_PACKAGE_NAME" > "${NSIS_BUILD_DIR}/buddy_src.txt"
echo -n "$WIN_REPO$EXECUTOR_PACKAGE_NAME" > "${NSIS_BUILD_DIR}/executor_src.txt"
echo -n "$WIN_REPO$MCL_PACKAGE_NAME" > "${NSIS_BUILD_DIR}/mcl_src.txt"

echo -e "${GREEN}Run nsis build$NC"
makensis "$NSIS_BUILD_DIR/nsisscript.nsi"

COMMIT_DATE=$(git show -s --date=short --format=%cd a5a0daeb | sed 's/-//g' | cut -c 3-8)
MANTICORE_VERSION=$(cat src/sphinxversion.h.in | grep VERNUMBERS | cut -d'"' -f2)

if [ -f "$NSIS_BUILD_DIR/manticore_installer.exe" ]; then
  echo "mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe"
  mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe
fi

