#!/bin/bash
manticore=0
manticore_port=9306

manql()
{
    echo "manql $1"
    mysql -h$manticore -P$manticore_port -e "$1"
}

load()
{
    echo "load $@"
    php -d memory_limit=2G ./test/clt-tests/optimisation-and-upgrade/load_names_attr.php $manticore $manticore_port $@
}

manql "drop table if exists name; create table name(username text, s string) min_infix_len='2' expand_keywords='1' rt_mem_limit='512m'"
load 10000 10 2000000 1 1
manql "flush ramchunk name"
load 10000 10 2000000 1 2000001
manql "flush ramchunk name"
load 10000 10 2000000 1 4000001
manql "flush rtindex name"

#php load_names_attr.php $manticore $manticore_port 10000 10 2000000 1 1
