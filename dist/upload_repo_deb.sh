#!/bin/bash
# from outside: $DISTRO = jessie,stretch,buster,trusty,xenial,bionic,focal,jammy
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

echo "Collected debian packages"
ls -1

if [ -z "${REPO_NAME}" ] || [ $REPO_NAME == '$REPO_NAME' ]; then
  echo "no repo was specified. Set default to manticoresearch"
  REPO_NAME="manticoresearch"
fi


for filename in *deb; do
  if [ -f "$filename" ]; then
    if [ -z "${IS_RELEASE_DIGIT}" ]; then
      IS_RELEASE_DIGIT=$(echo $filename | cut -d. -f3 | cut -d- -f1)
      if [[ $(($IS_RELEASE_DIGIT % 2)) -ne 0 ]]; then
        SUFFIX="_dev"
      fi
    fi

    f="${filename##*/}"
    ~/sign_deb.sh $GPG_SECRET $f
    curl -is --user "${REPO_USER}:${REPO_SECRET}" -H "Content-Type: multipart/form-data" --data-binary "@./$filename" $REPO_IP/repository/manticoresearch_$DISTRO$SUFFIX/ \
    && echo "Uploaded $f to manticoresearch_$DISTRO$SUFFIX"


    arch="amd"
    if [[ ! $f == *"${arch}64"* ]]; then
      arch="arm"
    fi

    echo -e "Copy $f to /mnt/repo_storage/manticoresearch_$DISTRO$SUFFIX/dists/$DISTRO/main/binary-${arch}64/";
    cp $f /mnt/repo_storage/manticoresearch_$DISTRO$SUFFIX/dists/$DISTRO/main/binary-${arch}64/ && echo -e "Success"
    echo -e "\n"
  fi
done

if [ ! -z $SUFFIX ]; then
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture amd -dev 1
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture arm -dev 1
else
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture amd
  /usr/bin/docker exec repo-generator /generator.sh -distro $DISTRO -architecture arm
fi

rm -rf *deb
