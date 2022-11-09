#!/bin/bash

# file included from all debian/ubuntu  in_*.sh in context of target system docker
# $distr and $arch should be declared on inclusion of this file

fixlinks()
{
  cd $1
  echo "fix links in $1"
  for a in *; do
    if [[ $(file $a | grep "symbolic link to /") ]]; then
      target=$(realpath --relative-to=. $a)
      ln -sf $target $a
      echo "relink $a -> $target"
    fi
  done
}

fixup()
{
  if [[ $(file $1 | grep "ASCII") ]]; then
    echo "fixup $1"
    sed -e 's+ \(/[^ ]*\)+ \n\1\n+g;' $1 |
    {
      while read i; do
      if [[ $i =~ ^/[^*] ]]; then
          realpath --relative-to=. $i
      else
          echo "$i"
      fi
    done
    } > tmp.tmp
    mv tmp.tmp $1
  fi
}

fixup_all()
{
  for a in *.so; do
    fixup $a
  done
  for a in *.a; do
    fixup $a
  done
}

apt-get -y install file

echo "fixup links"
fixlinks /usr/lib/$arch-linux-gnu
for a in /usr/lib/gcc/$arch-linux-gnu/*; do
  fixlinks $a
done

echo "fixup ld scripts"
cd /usr/lib/$arch-linux-gnu
fixup_all

symlink=$(file /lib | grep "symbolic link to usr/lib")
apt-get -y remove file
apt-get -y autoremove

echo "pack sysroot"
if [[ $symlink ]]; then
  echo "pack symlink"
  tar -cf - /usr/lib/$arch-linux-gnu /usr/lib/gcc /usr/include> /sysroot/$arch/sysroot_${distr}_$arch.tar
else
  echo "pack /lib folder"
  tar -cf - /usr/lib/$arch-linux-gnu /usr/lib/gcc /usr/include /lib/$arch-linux-gnu> /sysroot/$arch/sysroot_${distr}_$arch.tar
fi