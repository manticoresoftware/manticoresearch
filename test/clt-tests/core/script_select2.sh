#!/bin/bash
set -x

docker stop manticore
docker rm manticore 
docker run -e EXTRA=1 --name manticore -v /Users/pavelshilin/Desktop/WORK/manticore_search/manticoresearch/test/clt-tests/mysqldump/scripts/generate-1m-records.sh:/generate-1m-records.sh -d manticoresearch/manticore
sleep 5
docker exec manticore bash -c "/generate-1m-records.sh | mysql -P9306 -h0"
docker exec manticore mysql -P9306 -h0 -e "SHOW TABLES; SELECT COUNT(*) FROM t; SELECT * FROM t;"
