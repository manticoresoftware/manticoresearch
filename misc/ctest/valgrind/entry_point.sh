trap "mysqladmin shutdown" TERM
mysqld &
exec "$@"
wait