#!/bin/bash
# from outside: $DISTRO = jessie,stretch,buster,trusty,xenial,bionic,focal,jammy
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

repo_generator() {
   /usr/bin/docker exec -i repo-generator /generator.sh $@
}


echo "Collected debian packages in build/"
ls -1 build/

bundleamd=0
bundlearm=0

for f in build/*deb; do

  FILE_NAME="$(basename $f)"

  VER=$(echo $f | cut -d_ -f2)
  ARCH=$(echo $f | cut -d_ -f3 | cut -d. -f1)
  if [ -f "$f" ]; then
    if [ -z "${IS_RELEASE_DIGIT}" ]; then
      IS_RELEASE_DIGIT=$(echo $f | cut -d. -f3 | cut -d- -f1)
      if [[ $(($IS_RELEASE_DIGIT % 2)) -ne 0 ]]; then
          TARGET="dev"
          SUFFIX="_dev"
        else
          TARGET="release"
          SUFFIX=""
      fi
    fi

    # check if we really need to put icudata to the repo this time
    # notice: it still goes to bundle archives anyway
    if [[ $f == *icudata* ]]; then
      if [[ $PACK_ICUDATA == 1 ]]; then
        ARCH=all
      else
        ARCH=none
      fi
    fi

    if [[ $ARCH == "amd64" ]]; then
      repo_generator --distro $DISTRO --architecture amd --target $TARGET --name $FILE_NAME --not-index < $f
      bundleamd=1
    fi

    if [[ $ARCH == "arm64" ]]; then
      repo_generator --distro $DISTRO --architecture arm --target $TARGET --name $FILE_NAME --not-index < $f
      bundlearm=1
    fi

    if [[ $ARCH == "all" ]]; then
      repo_generator --distro $DISTRO --architecture amd --target $TARGET --name $FILE_NAME --not-index < $f
      repo_generator --distro $DISTRO --architecture arm --target $TARGET --name $FILE_NAME --not-index < $f
    fi
  fi
done

echo "make bundle(s) and upload them"

if [ $bundleamd == 1 ]; then
  echo Pack amd packages
  TGZ1=manticore_${VER}_amd64.tgz
  (cd build && tar cf - *_all.deb *_amd64.deb *icudata*deb | gzip -9 -f) > $TGZ1
  repo_generator --path "/repository/manticoresearch_$DISTRO$SUFFIX/dists/" --name $TGZ1 --not-index --skip-signing < $TGZ1
fi

if [ $bundlearm == 1 ]; then
  echo Pack arm packages
  TGZ2=manticore_${VER}_arm64.tgz
  (cd build && tar cf - *_all.deb *_arm64.deb *icudata*deb | gzip -9 -f) > $TGZ2
  repo_generator --path "/repository/manticoresearch_$DISTRO$SUFFIX/dists/" --name $TGZ2 --not-index --skip-signing < $TGZ2
fi

repo_generator --distro $DISTRO --architecture amd --target $TARGET --only-index
repo_generator --distro $DISTRO --architecture arm --target $TARGET --only-index

rm -rf build/*deb
