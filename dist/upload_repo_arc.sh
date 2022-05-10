#!/bin/bash
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

get_destination() {
  if [ -z "${IS_RELEASE_DIGIT}" ]; then
    IS_RELEASE_DIGIT=$(echo $f | cut -d. -f3 | cut -d- -f1)
    if [[ $(($IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
      DESTINATION="release"
    else
      DESTINATION="dev"
    fi
  fi
}

echo "Collected archives"
ls -1 build/

for f in build/*.zip; do
  if [ -f "$f" ]; then
    get_destination

    echo -e "Copy $f to /mnt/repo_storage/manticoresearch_windows/$DESTINATION/x64/";
    cp $f /mnt/repo_storage/manticoresearch_windows/$DESTINATION/x64/ && echo -e "Success"
    echo -e "\n"

  fi
done

for f in build/*.gz; do
  if [ -f "$f" ]; then
    get_destination

    echo -e "Copy $f to /mnt/repo_storage/manticoresearch_macos/$DESTINATION/";
    cp $f /mnt/repo_storage/manticoresearch_macos/$DESTINATION/ && echo -e "Success"
    echo -e "\n"

  fi
done

rm -rf build/*.zip
rm -rf build/*.gz
