# An entry point for docker

#cp -R /work/.ssh /root/

trap "mysqladmin shutdown" TERM
mysqld &
# sleep is necessary only in short-term run (without build)
#sleep 3

exec "$@"

wait