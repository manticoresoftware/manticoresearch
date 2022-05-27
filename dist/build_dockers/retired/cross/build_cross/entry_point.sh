# An entry point for docker

echo "Arch is $arch"
echo "Distr is $DISTR"

cd /sysroot/root
echo "Unpacking boost..."
tar -Jxf ../boost_${DISTR}_${arch}.tar.xz
echo "Unpacking sysroot..."
tar -Jxf ../sysroot_${DISTR}_${arch}.tar.xz
echo "Unpacking done"
cd -

exec "$@"

wait