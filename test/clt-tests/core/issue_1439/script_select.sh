#!/bin/bash
set -x

docker stop manticore
docker rm manticore 
docker run -e EXTRA=1 --name manticore -v /Users/pavelshilin/Desktop/WORK/manticoresearch/test/clt-tests/mysqldump/scripts/generate-records.sh:/generate-1m-records.sh -d manticoresearch/manticore
sleep 5
docker exec manticore bash -c "/generate-1m-records.sh 1000000 | mysql -P9306 -h0"
docker exec manticore mysql -P9306 -h0 -e "SHOW TABLES; SELECT COUNT(*) FROM t;"
docker exec manticore mysql -P9306 -h0 -e "SHOW TABLES; SELECT COUNT(*) FROM t; SELECT * FROM t;"
docker exec manticore mysql -P9306 -h0 -e "SELECT * FROM t ORDER BY id DESC;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, b FROM t WHERE id BETWEEN 10000 AND 50000;"
docker exec manticore mysql -P9306 -h0 -e "SELECT s, COUNT(*) FROM t GROUP BY s;"
docker exec manticore mysql -P9306 -h0 -e "SELECT a, COUNT(*) FROM t WHERE a > 100000 GROUP BY a HAVING COUNT(*) > 10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT * FROM t LIMIT 10 OFFSET 20;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, JSON_UNQUOTE(JSON_EXTRACT(j, '$.a')) FROM t WHERE MATCH('@j.a \"200000\"') ORDER BY id DESC LIMIT 5;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, a, b FROM t WHERE b IS NOT NULL ORDER BY b DESC LIMIT 10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT s, SUM(b) as total_b FROM t GROUP BY s HAVING total_b > 1000000;"
docker exec manticore mysql -P9306 -h0 -e "SELECT * FROM t WHERE MATCH('aaa') ORDER BY id DESC LIMIT 10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, b FROM t WHERE id IN (10001, 10005, 10010);"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, b, s FROM t WHERE MATCH('@f aa*') AND b BETWEEN 100000000 AND 200000000 ORDER BY b;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, JSON_UNQUOTE(JSON_EXTRACT(j, '$.b')) AS json_field FROM t WHERE MATCH('@j.b \"1000000\"') LIMIT 5;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, s FROM t WHERE MATCH('content:(+important -trivial)') LIMIT 10 OFFSET 5;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, b, s FROM t LIMIT 20,10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, s FROM t WHERE a BETWEEN 50000 AND 150000 GROUP BY s HAVING COUNT(*) > 5;"
docker exec manticore mysql -P9306 -h0 -e "SELECT a, GROUP_CONCAT(f SEPARATOR ', ') FROM t GROUP BY a HAVING COUNT(*) > 10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, a, b, s FROM t WHERE id MOD 2 = 0 ORDER BY id LIMIT 10;"
docker exec manticore mysql -P9306 -h0 -e "SELECT id, f, JSON_UNQUOTE(JSON_EXTRACT(j, '$.a')) AS json_field FROM t WHERE JSON_LENGTH(j, '$.a') > 1 ORDER BY JSON_LENGTH(j, '$.a') DESC LIMIT 5;"
