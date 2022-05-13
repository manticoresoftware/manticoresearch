# ---------- rhel8 ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat linux 8 / Centos 8

message ( STATUS "Will create RPM for RedHat/Centos 8" )

set (SYSTEMD_KILLMODE "process")
set ( RELEASE_DIST ".el8" )
include ( builds/CommonRpm )