source min {
    type = pgsql
    sql_host = postgres
    sql_user = postgres
    sql_pass = postgres
    sql_db = postgres
    sql_port = 5432
    sql_query = select 1, 'abc' title
}

index idx {
    path = /tmp/idx
    source = min
}

searchd {
    listen = 127.0.0.1:9315
    log = sphinx_min.log
    pid_file = /tmp/9315.pid
    binlog_path =
}
