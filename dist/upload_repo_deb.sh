#!/bin/bash
# from outside: $DISTRO = jessie,stretch,buster,trusty,xenial,bionic,focal,jammy
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

echo "Collected debian packages in build/"
ls -1 build/

copy_to() {
  echo -e "Copy $1 to /mnt/repo_storage/manticoresearch_$DISTRO$SUFFIX/dists/$2";
  cp $1 /mnt/repo_storage/manticoresearch_$DISTRO$SUFFIX/dists/$2 && echo -e "Success"
  echo
}

bundleamd=0
bundlearm=0

for f in build/*deb; do
  VER=$(echo $f | cut -d_ -f2)
  ARCH=$(echo $f | cut -d_ -f3 | cut -d. -f1)
  if [ -f "$f" ]; then
    if [ -z "${IS_RELEASE_DIGIT}" ]; then
      IS_RELEASE_DIGIT=$(echo $f | cut -d. -f3 | cut -d- -f1)
      if [[ $(($IS_RELEASE_DIGIT % 2)) -ne 0 ]]; then
        SUFFIX="_dev"
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

    ~/sign_deb.sh $GPG_SECRET $f

    if [[ $ARCH == "amd64" ]]; then
      copy_to $f $DISTRO/main/binary-amd64/
      bundleamd=1
    fi

    if [[ $ARCH == "arm64" ]]; then
      copy_to $f $DISTRO/main/binary-arm64/
      bundlearm=1
    fi

    if [[ $ARCH == "all" ]]; then
      copy_to $f $DISTRO/main/binary-arm64/
      copy_to $f $DISTRO/main/binary-amd64/
    fi
  fi
done

echo "make bundle(s) and upload them"

if [ $bundleamd == 1 ]; then
  echo Pack amd packages
  TGZ1=manticore_${VER}_amd64.tgz
  (cd build && tar cf - *_all.deb *_amd64.deb *icudata*deb | gzip -9 -f) > $TGZ1
  copy_to $TGZ1
fi

if [ $bundlearm == 1 ]; then
  echo Pack arm packages
  TGZ2=manticore_${VER}_arm64.tgz
  (cd build && tar cf - *_all.deb *_arm64.deb *icudata*deb | gzip -9 -f) > $TGZ2
  copy_to $TGZ2
fi

if [ ! -z $SUFFIX ]; then
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture amd -dev 1
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture arm -dev 1
else
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture amd
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture arm
fi

rm -rf build/*deb
