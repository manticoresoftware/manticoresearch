#!/bin/bash
# from outside: $DISTRO = 6,7,8
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

echo "Collected rpm packages"
ls -1 build/

bundleaarch=0
bundleintel=0

for f in build/*.rpm; do
  echo file $f
  tail=$(echo $f | sed 's_build/__g;s/[a-z]*-//g;')
  VER=$(echo $tail | cut -d. -f1,2,3,4,5)
  if [[ $tail == *".x86_64."* ]]; then
    ARCH=x86_64
  elif [[ $tail == *".aarch64."* ]]; then
    ARCH=aarch64
  elif [[ $tail == *".noarch."* ]]; then
    ARCH=noarch
  fi;
  if [ -f "$f" ]; then
    if [ -z "${IS_RELEASE_DIGIT}" ]; then
    IS_RELEASE_DIGIT=$(echo $f | cut -d. -f3 | cut -d_ -f1)
      if [[ $(($IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
        DESTINATION="release"
      else
        DESTINATION="dev"
      fi
    fi

    # check if we really need to put icudata to the repo this time
    # notice: it still goes to bundle archives anyway
    if [[ $f == *icudata* ]]; then
      if [[ $PACK_ICUDATA == 1 ]]; then
        ARCH=noarch
      else
        # "None" is a nonexisting architecture. In case it's set we build bundle (zip) without building rpm
        # more answers can give @klirichek
        ARCH=none
      fi
    fi

    if [[ ! $ARCH == "none" ]]; then
          if [[ $ARCH == "x86_64" ]]; then
            /usr/bin/docker exec -i repo-generator /generator.sh -n $(basename $f) -d centos -v $DISTRO -t $DESTINATION -a x86_64 --not-index < $f
            bundleintel=1
          fi

          if [[ $ARCH == "aarch64" ]]; then
            /usr/bin/docker exec -i repo-generator /generator.sh -n $(basename $f) -d centos -v $DISTRO -t $DESTINATION -a aarch64 --not-index < $f
            bundleaarch=1
          fi

          if [[ $ARCH == "noarch" ]]; then
            /usr/bin/docker exec -i repo-generator /generator.sh -n $(basename $f) -d centos -v $DISTRO -t $DESTINATION -a x86_64 --not-index < $f
            /usr/bin/docker exec -i repo-generator /generator.sh -n $(basename $f) -d centos -v $DISTRO -t $DESTINATION -a aarch64 --not-index < $f
          fi
    fi

  fi
done

# upload bundle(s)

if [ $bundleintel == 1 ]; then
  echo make bundle x86_64
  TGZ1=manticore-${VER}.x86_64.tgz
  (cd build && tar cf - $(ls | grep -v -e debuginfo | grep "x86_64\|noarch") *icudata*rpm | gzip -9 -f) > $TGZ1
  /usr/bin/docker exec -i repo-generator /generator.sh -p "/repository/manticoresearch/$DESTINATION/centos/$DISTRO/" -n $TGZ1 --not-index --skip-signing < $TGZ1
fi

if [ $bundleaarch == 1 ]; then
  echo make bundle aarch64
  TGZ2=manticore-${VER}.aarch64.tgz
  (cd build && tar cf - $(ls | grep -v -e debuginfo | grep "aarch64\|noarch") *icudata*rpm | gzip -9 -f) > $TGZ2
  /usr/bin/docker exec -i repo-generator /generator.sh -p "/repository/manticoresearch/$DESTINATION/centos/$DISTRO/" -n $TGZ2 --not-index --skip-signing < $TGZ2
fi

/usr/bin/docker exec -i repo-generator /generator.sh -d centos -v $DISTRO --architecture aarch64 --target $DESTINATION --only-index
/usr/bin/docker exec -i repo-generator /generator.sh -d centos -v $DISTRO --architecture x86_64 --target $DESTINATION --only-index


rm -rf build/*.rpm
