#!/bin/bash

fixlinks()
{
  cd $1
  echo "fix links in $1"
  for a in *; do
    if [[ $(file $a | grep "symbolic link to \`\?/") ]]; then
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
    mv -f tmp.tmp $1
  fi
}

fixup_all()
{
  cd $1
  echo "fixup ld scripts in $1"
  for a in *.so; do
    fixup $a
  done
  for a in *.a; do
    fixup $a
  done
}

yum install -y file

test -d /opt/rh/devtoolset-8/root && cd $_ && tar -cf - usr/include/c++ usr/lib> /tmp.tar && cd / && tar -xf tmp.tar
sleep 1

echo "fixup links"
fixlinks /usr/lib64
for a in /usr/lib/gcc/$arch-redhat-linux/*; do
  fixlinks $a
done

echo "fixup ld scripts"
fixup_all /usr/lib64
for a in /usr/lib/gcc/$arch-redhat-linux/*; do
  fixup_all $a
done

yum -y remove file
yum -y autoremove

echo "pack sysroot"
tar -cf - /usr/lib/gcc /usr/lib64 /usr/include /lib /lib64> /sysroot/$arch/sysroot_${distr}_$arch.tar
