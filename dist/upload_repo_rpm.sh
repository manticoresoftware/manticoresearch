#!/bin/bash
# from outside: $DISTRO = 6,7,8
# That file here is for reference; actually used the one stored on the host to avoid checkout of the whole code

echo "Collected rpm packages"
ls -1

if [ -z "${REPO_NAME}" ] || [ $REPO_NAME == '$REPO_NAME' ]; then
  echo "no repo was specified. Set default to manticoresearch"
  REPO_NAME="manticoresearch"
fi

#force 'release' for Ivinco-testing
if  [ $REPO_NAME == 'Ivinco-testing' ]; then
  DESTINATION="release"
fi

for filename in *.rpm; do
  if [ -f "$filename" ]; then
    if [ -z "${IS_RELEASE_DIGIT}" ]; then
    IS_RELEASE_DIGIT=$(echo $filename | cut -d. -f3 | cut -d_ -f1)
      if [[ $(($IS_RELEASE_DIGIT % 2)) -eq 0 ]]; then
        DESTINATION="release"
      else
        DESTINATION="dev"
      fi
    fi

    f="${filename##*/}"
    ~/sign_rpm.sh $GPG_SECRET $f
    curl -is --user "${REPO_USER}:${REPO_SECRET}" --upload-file $filename $REPO_IP/repository/$REPO_NAME/$DESTINATION/centos/$DISTRO/x86_64/$f \
    && echo "Uploaded $f to $REPO_NAME/$DESTINATION/centos/$DISTRO/x86_64"

    echo -e "Copy $f to /work/repomanager/repodata/repository/manticoresearch/$DESTINATION/centos/$DISTRO/x86_64/";
    cp $f /work/repomanager/repodata/repository/manticoresearch/$DESTINATION/centos/$DISTRO/x86_64/ && echo -e "Success"
    echo -e "\n"
  fi
done

if [ "$DESTINATION" = "dev" ]; then
    /usr/bin/docker exec repo-generator /generator.sh -distro centos -version $DISTRO -dev 1
  else
    /usr/bin/docker exec repo-generator /generator.sh -distro centos -version $DISTRO
fi

rm -rf *.rpm
