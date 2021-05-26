# ---------- debian ----------
# Above line is mandatory!
# rules to build general deb package for Debian

SET ( NOAPI ON )

message ( STATUS "Will create DEB for Debian (sid)" )

set (flavour "debian")
include ( builds/CommonDeb )
