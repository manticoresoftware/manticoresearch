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
  echo "Fetching boost headers..."
  wget $SYSROOT_URL/$boost/boost_include.tar.xz -q
  echo "Unpacking boost headers..."
  tar -xf boost_include.tar.xz
  cd /sysroot/root
  echo "Fetching boost..."
  wget $SYSROOT_URL/$boost/boost_${DISTR}_${arch}.tar.xz -q
  echo "Unpacking boost..."
  tar -xf boost_${DISTR}_${arch}.tar.xz
fi

cd /sysroot/root
echo "Fetching sysroot..."
wget $SYSROOT_URL/$sysroot/sysroot_${DISTR}_${arch}.tar.xz -q
echo "Unpacking sysroot..."
tar -xf sysroot_${DISTR}_${arch}.tar.xz
echo "Preparing boost and sysroot done"
echo 'PS1="\[\e]0;\u@\h: \w\a\]${debian_chroot:+($debian_chroot)}\u@\h:\w \[\033[1;31m\][llvm-${llvm} ${DISTR} ${arch}]\[\033[0m\]\$ "' >> ~/.bashrc
exec "$@"
wait