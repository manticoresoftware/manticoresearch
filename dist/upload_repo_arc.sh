#!/bin/bash
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

repo_generator() {
   /usr/bin/docker exec -i repo-generator /generator.sh $@
}


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

# check if we really need to pack icudata this time
if [[ $PACK_ICUDATA == 1 ]]; then
  echo -e "Pack icudata, since PACK_ICUDATA is set to 1"
else
  rm build/manticore-icudata-65l.*
fi

for f in build/*.zip build/*.exe; do
  if [ -f "$f" ]; then
    get_destination
    repo_generator --path "/repository/manticoresearch_windows/$DESTINATION/x64/" --name $(basename $f) --not-index --skip-signing < $f
  fi
done

for f in build/*.gz; do
  if [ -f "$f" ]; then
    get_destination
    repo_generator --path "/repository/manticoresearch_macos/$DESTINATION/" --name $(basename $f) --not-index --skip-signing < $f
  fi
done

rm -rf build/*.zip
rm -rf build/*.exe
rm -rf build/*.gz
