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
MANTICORE_DATE=$(TZ=UTC git log -1 --date=iso-strict-local --format=%cd)
MANTICORE_DATE=${MANTICORE_DATE//-/}
MANTICORE_DATE=${MANTICORE_DATE//T/}
MANTICORE_DATE=${MANTICORE_DATE:2:8}

MANTICORE_FULL_VERSION="manticore-${MANTICORE_VERSION}-${MANTICORE_DATE}-${CI_COMMIT_SHORT_SHA}-x64-main.zip"
echo "MANTICORE_FULL_VERSION: $MANTICORE_FULL_VERSION"

IS_RELEASE_DIGIT=$(echo "$MANTICORE_VERSION" | cut -d. -f3)
if [[ $((IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
  DESTINATION_REPOS=("release" "release_candidate")
  MANTICORE_PACKAGE_NAME="https://repo.manticoresearch.com/repository/manticoresearch_windows/release_candidate/x64/$MANTICORE_FULL_VERSION"
else
  DESTINATION_REPOS=("dev")
  MANTICORE_PACKAGE_NAME="https://repo.manticoresearch.com/repository/manticoresearch_windows/dev/x64/$MANTICORE_FULL_VERSION"
fi

echo "MANTICORE_PACKAGE_NAME: $MANTICORE_PACKAGE_NAME"

DEPS_PATH="deps.txt"

OLD_IFS=$IFS
IFS=' +-'
read BUDDY_PACKAGE_NAME BUDDY_VERSION BUDDY_DATE BUDDY_COMMIT_SHA <<< "$(grep buddy $DEPS_PATH)"
read MCL_PACKAGE_NAME MCL_VERSION MCL_DATE MCL_COMMIT_SHA <<< "$(grep mcl $DEPS_PATH)"
read EXECUTOR_PACKAGE_NAME EXECUTOR_VERSION EXECUTOR_DATE EXECUTOR_COMMIT_SHA <<< "$(grep executor $DEPS_PATH)"
read TZDATA_PACKAGE_NAME TZDATA_VERSION TZDATA_DATE TZDATA_COMMIT_SHA <<< "$(grep tzdata $DEPS_PATH)"
IFS=$OLD_IFS

echo "Parsed from deps:
$BUDDY_PACKAGE_NAME $BUDDY_VERSION $BUDDY_COMMIT_SHA $BUDDY_DATE
$MCL_PACKAGE_NAME $MCL_VERSION $MCL_COMMIT_SHA $MCL_DATE
$EXECUTOR_PACKAGE_NAME $EXECUTOR_VERSION $EXECUTOR_COMMIT_SHA $EXECUTOR_DATE
$TZDATA_PACKAGE_NAME $TZDATA_VERSION $TZDATA_COMMIT_SHA $TZDATA_DATE"

find_package_by_commit() {
  local repo_content=$1
  local win_repo=$2
  local commit_sha=$3

  echo "$repo_content" \
    | grep "$commit_sha" \
    | grep -Eo 'href="[^"]*\.zip"' \
    | sed 's|href="||;s|"||' \
    | awk -v repo="$WIN_REPO" '
        {
            if (/main\.zip$/)           { p=3 }
            else if (/libs\.zip$/)      { p=2 }
            else if (/\.zip$/)          { p=1 }
            if (p > priority) { priority=p; best=$0 }
        }
        END { if (best) print repo best; exit !best }
    '

}

for DESTINATION in "${DESTINATION_REPOS[@]}"; do
  echo "Commit: $CI_COMMIT_SHORT_SHA, Version: $MANTICORE_VERSION, Search in $DESTINATION repo"

  WIN_REPO="https://repo.manticoresearch.com/repository/manticoresearch_windows/$DESTINATION/x64/"

  REPO_CONTENT=$(curl -s "$WIN_REPO")

  BUDDY_PACKAGE_NAME=$(find_package_by_commit "$REPO_CONTENT" "$WIN_REPO" "$BUDDY_COMMIT_SHA")
  MCL_PACKAGE_NAME=$(find_package_by_commit "$REPO_CONTENT" "$WIN_REPO" "$MCL_COMMIT_SHA")
  TZDATA_PACKAGE_NAME=$(find_package_by_commit "$REPO_CONTENT" "$WIN_REPO" "$TZDATA_COMMIT_SHA")
  echo "Found packages: BUDDY_PACKAGE_NAME: $BUDDY_PACKAGE_NAME
  MCL_PACKAGE_NAME: $MCL_PACKAGE_NAME
  TZDATA_PACKAGE_NAME: $TZDATA_PACKAGE_NAME"

  if [ -n "$BUDDY_PACKAGE_NAME" ] && [ -n "$MCL_PACKAGE_NAME" ] && [ -n "$MANTICORE_PACKAGE_NAME" ] && [ -n "$TZDATA_PACKAGE_NAME" ] && [ -n "$EXECUTOR_VERSION" ]; then
    echo "All required packages found in $DESTINATION repo / deps.txt"
    break
  else
    echo "Some packages are missing in $DESTINATION repo / deps.txt"
    BUDDY_PACKAGE_NAME=""
    MCL_PACKAGE_NAME=""
    TZDATA_PACKAGE_NAME=""
  fi
done

if [ -z "$BUDDY_PACKAGE_NAME" ] || [ -z "$MCL_PACKAGE_NAME" ] || [ -z "$MANTICORE_PACKAGE_NAME" ] || [ -z "$TZDATA_PACKAGE_NAME" ] || [ -z "$EXECUTOR_VERSION" ]; then
  [ -z "$BUDDY_PACKAGE_NAME" ] && echo -e "${RED}Can't find Buddy package with commit $BUDDY_COMMIT_SHA in repositories. Exiting$NC \n"
  [ -z "$MCL_PACKAGE_NAME" ] && echo -e "${RED}Can't find Columnar package with commit $MCL_COMMIT_SHA in repositories. Exiting$NC \n"
  [ -z "$MANTICORE_PACKAGE_NAME" ] && echo -e "${RED}Can't set Manticore package with commit $CI_COMMIT_SHORT_SHA. Exiting$NC \n"
  [ -z "$TZDATA_PACKAGE_NAME" ] && echo -e "${RED}Can't find TZDATA package with commit $TZDATA_COMMIT_SHA in repositories. Exiting$NC \n"
  [ -z "$EXECUTOR_VERSION" ] && echo -e "${RED}Can't find Executor in deps.txt. Exiting$NC \n"
  exit 1
fi

echo -e "${GREEN}Save src to $NSIS_BUILD_DIR $NC"

echo -n "$MANTICORE_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/manticore_src.txt"
echo -n "$BUDDY_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/buddy_src.txt"
echo -n "manticoresearch/manticore-executor:$EXECUTOR_VERSION" >"${NSIS_BUILD_DIR}/executor_src.txt"
echo -n "$MCL_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/mcl_src.txt"
echo -n "$TZDATA_PACKAGE_NAME" >"${NSIS_BUILD_DIR}/tzdata_src.txt"

echo -e "${GREEN}Run nsis build$NC"
makensis "$NSIS_BUILD_DIR/nsisscript.nsi"

COMMIT_DATE=$(git show -s --date=format-local:"%y%m%d%H" --format=%cd $CI_COMMIT_SHORT_SHA)

if [ -f "$NSIS_BUILD_DIR/manticore_installer.exe" ]; then
  echo "mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe"
  mv $NSIS_BUILD_DIR/manticore_installer.exe /builds/manticoresearch/dev/build/manticore-$MANTICORE_VERSION-$COMMIT_DATE-$CI_COMMIT_SHORT_SHA-x64.exe
fi