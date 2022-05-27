# Extracting sysroot

Follow instructions on https://github.com/tpoechtrager/osxcross#packaging-the-sdk

Store that archive and put aside Dockerfile.

If you have another version of SDK (not 11.3) you need also to correct CMAKE_SYSROOT in toolchain.cmake

Prepared archive is also stored on our dev server, in `/work/docker` folder.