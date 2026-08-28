# ---------- rhel10 ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat linux 10 / Almalinux 10 / Centos Stream 10

message ( STATUS "Will create RPM for RedHat/Centos Stream 10" )

set ( SYSTEMD_KILLMODE "process" )
set ( RELEASE_DIST ".el10" )
include ( builds/CommonRpm )