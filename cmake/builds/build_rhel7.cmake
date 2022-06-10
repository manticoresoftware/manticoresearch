# ---------- rhel7 ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat linux 7 / Centos 7

message ( STATUS "Will create RPM for RedHat/Centos 7" )

set ( SYSTEMD_KILLMODE "none" )
set ( RELEASE_DIST ".el7.centos" )
include ( builds/CommonRpm )