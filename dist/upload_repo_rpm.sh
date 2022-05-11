#!/bin/bash
# from outside: $DISTRO = 6,7,8
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

echo "Collected rpm packages"
ls -1 build/

#force 'release' for Ivinco-testing
if  [ $REPO_NAME == 'Ivinco-testing' ]; then
  DESTINATION="release"
fi

copy_to() {
    echo -e "Copy $1 to /mnt/repo_storage/manticoresearch/$DESTINATION/centos/$DISTRO/$2";
    cp $1 /mnt/repo_storage/manticoresearch/$DESTINATION/centos/$DISTRO/$2 && echo -e "Success"
    echo -e "\n"
}

for f in build/*.rpm; do
  echo file $f
  tail=$(echo $f | sed 's_build/__g;s/[a-z]*-//g;')
  VER=$(echo $tail | cut -d. -f1,2,3,4,5)
  if [[ $tail == *".x86_64."* ]]; then
    ARCH=x86_64
  elif [[ $tail == *".arm64."* ]]; then
    ARCH=arm64
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

    ~/sign_rpm.sh $GPG_SECRET $f

    if [[ $ARCH == "x86_64" || $ARCH == "arm64" ]]; then
      copy_to $f $ARCH/
      arch=$ARCH
    fi

    if [[ $ARCH == "noarch" ]]; then
      copy_to $f x86_64/
#      copy_to $f arm64/
    fi

  fi
done

# make bundle
TGZ=manticore-${VER}.${ARCH}.tgz
(cd build && tar cf - $(ls | grep -v -e debuginfo) | gzip -9 -f) > $TGZ

# upload bundle
copy_to $TGZ

if [ "$DESTINATION" = "dev" ]; then
    /usr/bin/docker exec repo-generator /generator.sh -distro centos -version $DISTRO -dev 1
  else
    /usr/bin/docker exec repo-generator /generator.sh -distro centos -version $DISTRO
fi

rm -rf *.rpm
