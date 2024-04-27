#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ -z "$CI_COMMIT_SHA" ]; then
  echo -e "${RED}CI_COMMIT_SHA not passed. Exiting$NC \n"
  exit 1
fi

NSIS_BUILD_DIR=$(mktemp -d)

cp dist/build_dockers/nsis/nsisscript.nsi $NSIS_BUILD_DIR

CI_COMMIT_SHORT_SHA=${CI_COMMIT_SHA:0:9}

MANTICORE_VERSION=$(cat src/sphinxversion.h.in | grep VERNUMBERS | cut -d'"' -f2)
MANTICORE_DATE=$(git log -1 --date=iso-strict --format=%cd)
MANTICORE_DATE=${MANTICORE_DATE//-/}
MANTICORE_DATE=${MANTICORE_DATE//T/}
MANTICORE_DATE=${MANTICORE_DATE:2:8}

IS_RELEASE_DIGIT=$(echo "$MANTICORE_VERSION" | cut -d. -f3)
if [[ $((IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
  DESTINATION_REPOS=("release" "release_candidate")
else
  DESTINATION_REPOS=("dev")
fi

DEPS_PATH="deps.txt"

read BUDDY_PACKAGE_NAME BUDDY_VERSION BUDDY_DATE BUDDY_COMMIT_SHA <<< "$(grep buddy $DEPS_PATH)"
read MCL_PACKAGE_NAME MCL_VERSION MCL_DATE MCL_COMMIT_SHA <<< "$(grep mcl $DEPS_PATH)"
read EXECUTOR_PACKAGE_NAME EXECUTOR_VERSION EXECUTOR_DATE EXECUTOR_COMMIT_SHA <<< "$(grep executor $DEPS_PATH)"
read TZDATA_PACKAGE_NAME TZDATA_VERSION TZDATA_DATE TZDATA_COMMIT_SHA <<< "$(grep tzdata $DEPS_PATH)"

for DESTINATION in "${DESTINATION_REPOS[@]}"; do

  BUDDY_PACKAGE_NAME=""
  MCL_PACKAGE_NAME=""
  TZDATA_PACKAGE_NAME=""
  MANTICORE_PACKAGE_NAME=""

  echo "Commit: $CI_COMMIT_SHORT_SHA, Version: $MANTICORE_VERSION, Search in $DESTINATION repo"

  WIN_REPO="https://repo.manticoresearch.com/repository/manticoresearch_windows/$DESTINATION/x64/"

  BUDDY_PACKAGE_NAME="manticore-buddy_${BUDDY_VERSION}_${BUDDY_DATE}.${BUDDY_COMMIT_SHA}.zip"
  MCL_PACKAGE_NAME="manticore-columnar-lib-${MCL_VERSION}-${MCL_DATE}-${MCL_COMMIT_SHA}-x64-libs.zip"
  MANTICORE_PACKAGE_NAME="manticore-${MANTICORE_VERSION}-${MANTICORE_DATE}-${CI_COMMIT_SHORT_SHA}-x64-main.zip"
  TZDATA_PACKAGE_NAME="manticore-tzdata-${TZDATA_VERSION}-${TZDATA_DATE}-${TZDATA_COMMIT_SHA}-x64.zip"

  if [ -n "$MANTICORE_PACKAGE_NAME" ] && [ -n "$BUDDY_PACKAGE_NAME" ] && [ -n "$EXECUTOR_VERSION" ] && [ -n "$MCL_PACKAGE_NAME" ] && [ -n "$TZDATA_PACKAGE_NAME" ]; then
      echo "All selected packages are found in $DESTINATION repo"
      break
  fi

done

if [ -z "$MANTICORE_PACKAGE_NAME" ] || [ -z "$BUDDY_PACKAGE_NAME" ] || [ -z "$EXECUTOR_VERSION" ] || [ -z "$MCL_PACKAGE_NAME" ] || [ -z "$TZDATA_PACKAGE_NAME" ]; then
  [ -z "$MANTICORE_PACKAGE_NAME" ] && echo -e "${RED}Can't find Manticore package $CI_COMMIT_SHORT_SHA in repositories. Exiting$NC \n"
  [ -z "$BUDDY_PACKAGE_NAME" ] && echo -e "${RED}Can't find Buddy package $BUDDY_COMMIT_SHA in repositories. Exiting$NC \n"
  [ -z "$EXECUTOR_VERSION" ] && echo -e "${RED}Can't find Executor package $EXECUTOR_VERSION in repositories. Exiting$NC \n"
  [ -z "$MCL_PACKAGE_NAME" ] && echo -e "${RED}Can't find Columnar package $MCL_COMMIT_SHA in repositories. Exiting$NC \n"
  [ -z "$TZDATA_PACKAGE_NAME" ] && echo -e "${RED}Can't find TZDATA package $TZDATA_COMMIT_SHA in repositories. Exiting$NC \n"

  exit 1
fi

echo -e "${GREEN}Save src to temp files$NC"

echo -n "$WIN_REPO$MANTICORE_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/manticore_src.txt"
echo -n "$WIN_REPO$BUDDY_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/buddy_src.txt"
echo -n "manticoresearch/manticore-executor:$EXECUTOR_VERSION" >"${NSIS_BUILD_DIR}/executor_src.txt"
echo -n "$WIN_REPO$MCL_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/mcl_src.txt"
echo -n "$WIN_REPO$TZDATA_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/tzdata_src.txt"

echo -e "${GREEN}Run nsis build$NC"
makensis "$NSIS_BUILD_DIR/nsisscript.nsi"

COMMIT_DATE=$(git show -s --date=short --format=%cd $CI_COMMIT_SHORT_SHA | sed 's/-//g' | cut -c 3-8)

if [ -f "$NSIS_BUILD_DIR/manticore_installer.exe" ]; then
  echo "mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe"
  mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe
fi
