#!/bin/bash
# from outside: $DISTRO = jessie,stretch,buster,trusty,xenial,bionic,focal
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
  fi
done