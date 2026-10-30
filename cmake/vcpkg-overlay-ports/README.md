# OpenSSL security build input

`GetSSL.cmake` selects this overlay when installing OpenSSL through vcpkg.
It pins the dependency to **OpenSSL 3.6.5**, replacing the OpenSSL 3.6.1
recipe available in older build-toolchain images. A failed installation stops
configuration rather than selecting an older system or cached library.

The `openssl/` files are copied without content changes from Microsoft's
vcpkg OpenSSL 3.6.5 port:

- Repository: https://github.com/microsoft/vcpkg
- Commit: `e182cb4dd2df2ab02f66a1aabd5f35bbdc9522c7`
- Port Git tree: `ab6768c10c3809e9b2b97612fe84566ad1c7f4a7`
- Source: https://github.com/microsoft/vcpkg/tree/e182cb4dd2df2ab02f66a1aabd5f35bbdc9522c7/ports/openssl
- License: [Microsoft vcpkg MIT license](LICENSE.vcpkg.txt).

The port retains its upstream source SHA-512 check and platform patches.
Updating the port changes vcpkg's package ABI/cache key; existing 3.6.1
packages are not accepted as builds of this recipe. The remaining dependencies
continue to use the toolchain's existing registry.

## CVE-2026-84782

The [OpenSSL advisory of 29 September 2026](https://openssl-library.org/news/secadv/20260929.txt)
classifies this DTLS handshake retransmission flaw as **High**. In the 3.6
branch, versions 3.6.0 through 3.6.4 are affected; 3.6.5 contains the fix.
A retransmission while a fragmented handshake write is suspended can reuse a
stale buffer offset and overwrite the suspended write's bookkeeping. This can
disclose heap bytes to the peer or crash the process. The affected code is
outside the FIPS module boundary.

This is a DTLS-specific issue. The inspected Manticore listener uses
`TLS_server_method()` for TCP TLS; embedding an affected OpenSSL version does
not by itself establish a reachable DTLS attack path. The upgrade removes
the affected dependency regardless of that reachability assessment.

## Rebuild and verification

Builds without vcpkg continue to select OpenSSL from their configured provider.
The version check rejects the affected upstream 3.6 patch releases when SSL is
enabled. It is not a comprehensive security gate for other release branches or
distribution backports. Cross-compiled Windows bundles are not rebuilt by
`GetSSL.cmake`; their provider must supply an appropriate patched library.

Run `python3 test/openssl/test_build_dependency.py` for the configure checks.
Then build and test Manticore with the selected 3.6.5 libraries, checking the
actual library paths in CMake's cache/link commands and the version in the
resulting artifact. Header checks alone cannot prove that a binary was relinked.
Existing static `searchd` binaries require rebuilding and replacement; changing
the host's `openssl` executable or shared libraries cannot update embedded code.
