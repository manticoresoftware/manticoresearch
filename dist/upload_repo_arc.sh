#!/bin/bash
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

get_destination() {
  if [ -z "${IS_RELEASE_DIGIT}" ]; then
    IS_RELEASE_DIGIT=$(echo $filename | cut -d. -f3 | cut -d- -f1)
    if [[ $(($IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
      DESTINATION="release"
    else
      DESTINATION="dev"
    fi
  fi
}

echo "Collected archives"
ls -1

for filename in *.zip; do
  if [ -f "$filename" ]; then
    get_destination
    f="${filename##*/}"
    curl -is --user "${REPO_USER}:${REPO_SECRET}" --upload-file $filename $REPO_IP/repository/manticoresearch_windows/$DESTINATION/x64/$f \
    && echo "Uploaded $f to manticoresearch_windows/$DESTINATION/x64"
  fi
done

for filename in *.gz; do
  if [ -f "$filename" ]; then
    get_destination
    f="${filename##*/}"
    curl -is --user "${REPO_USER}:${REPO_SECRET}" --upload-file $filename $REPO_IP/repository/manticoresearch_macos/$DESTINATION/$f \
    && echo "Uploaded $f to manticoresearch_macos/$DESTINATION"
  fi
done