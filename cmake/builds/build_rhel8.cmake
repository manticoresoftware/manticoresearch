# ---------- rhel8 ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat linux 8 / Centos 8

message ( STATUS "Will create RPM for RedHat/Centos 8" )

set (CPACK_RPM_SERVER_PACKAGE_SUGGESTS "manticore-icudata")
set (CPACK_RPM_TOOLS_PACKAGE_SUGGESTS "manticore-icudata")
set (SYSTEMD_KILLMODE "process")
include ( builds/CommonRpm )