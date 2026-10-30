# OpenSSL 3.6.5 fixes CVE-2026-84782. Other release branches can carry
# distribution backports and must be evaluated against their vendor advisories.
if (NOT WITH_SSL)
	return()
endif()

# CMake 4.2 renamed OPENSSL_VERSION to OpenSSL_VERSION.
if (DEFINED OpenSSL_VERSION)
	set ( _openssl_version "${OpenSSL_VERSION}" )
else()
	set ( _openssl_version "${OPENSSL_VERSION}" )
endif()

if (_openssl_version VERSION_GREATER_EQUAL "3.6.0" AND _openssl_version VERSION_LESS "3.6.5")
	message ( FATAL_ERROR
		"OpenSSL ${_openssl_version} is affected by CVE-2026-84782. "
		"Use OpenSSL 3.6.5 or newer in the 3.6 series and rebuild statically linked binaries. "
		"See https://openssl-library.org/news/secadv/20260929.txt" )
endif()
unset ( _openssl_version )
