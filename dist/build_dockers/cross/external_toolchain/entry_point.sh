# An entry point for docker

echo "Arch is $arch"
echo "Distr is $DISTR"
echo "Sysroot URL is $SYSROOT_URL (configured most probably in gitlab variable)"

[ -v $boost ] && boost=boost_178
[ -v $sysroot ] && sysroot=roots_with_zstd

echo "Will use Boost from dir $boost on the server; may be set to 'none', or other value with boost=XYZ clause"
echo "Will use sysroot from dir $sysroot on the server; may be set to other value with sysroot=XYZ clause"

export sysroot
export boost

if [[ $boost != "none" ]]; then
  mkdir -p /sysroot/cache/common/boost && cd /sysroot/cache/common/boost
  echo "Fetching and unpacking boost headers..."
  curl -s -L $SYSROOT_URL/$boost/boost_include.tar.xz | tar -xJ
  cd /sysroot/root
  echo "Fetching and unpacking boost libraries..."
  curl -s -L $SYSROOT_URL/$boost/boost_${DISTR}_${arch}.tar.xz | tar -xJ
fi

cd /sysroot/root
echo "Fetching and unpacking sysroot..."
curl -s -L $SYSROOT_URL/$sysroot/sysroot_${DISTR}_${arch}.tar.xz | tar -xJ
echo "Preparing boost and sysroot done"
export debian_chroot="$DISTR $arch"
exec "$@"
wait
