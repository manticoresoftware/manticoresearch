# An entry point for docker

echo "Arch is $arch"
echo "Distr is $DISTR"
echo "Sysroot URL is $SYSROOT_URL, configured in gitlab variables"

cd /sysroot/root
echo "Fetching boost..."
wget $SYSROOT_URL/boost_178/boost_${DISTR}_${arch}.tar.xz -q
echo "Unpacking boost..."
tar -Jxf boost_${DISTR}_${arch}.tar.xz
echo "Fetching sysroot..."
wget $SYSROOT_URL/roots_with_zstd/sysroot_${DISTR}_${arch}.tar.xz -q
echo "Unpacking sysroot..."
tar -Jxf sysroot_${DISTR}_${arch}.tar.xz
echo "Preparing boost and sysroot done"

exec "$@"
wait