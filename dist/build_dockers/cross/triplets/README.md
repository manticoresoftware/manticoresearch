## VCPKG triplets

This stuf assume also following definitions:

```cmake
set ( ENV{VCPKG_OVERLAY_TRIPLETS} "${MANTICORE_SOURCE_DIR}/dist/build_dockers/cross/triplets" )
set ( ENV{MANTICORE_SOURCE_DIR} "${MANTICORE_SOURCE_DIR}" )
```

* env `VCPKG_OVERLAY_TRIPLETS` will automatically rule vcpkg here, so these triplets will be used instead of defaults.
* env `MANTICORE_SOURCE_DIR` is used in triplets themselves to initialize `VCPKG_CHAINLOAD_TOOLCHAIN_FILE` variable