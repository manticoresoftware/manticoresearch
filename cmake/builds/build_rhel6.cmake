# ---------- rhel6 ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat linux 6 / Centos 6

message ( STATUS "Will create RPM for RedHat/Centos 6" )
include ( builds/CommonRpm )
LIST ( APPEND PKGSUFFIXES "rhel6" )