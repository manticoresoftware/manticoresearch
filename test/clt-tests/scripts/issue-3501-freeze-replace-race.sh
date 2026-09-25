#!/bin/bash
# manticoresearch#3501: FREEZE arrives while a regular (not forced) disk chunk save is writing. A burst of small REPLACEs
# spans the FREEZE call; each touches one document of disk chunk 0 (so the commit pauses once saving is disabled) and one
# document of the RAM segments being saved. Prints how many of the replaced ids are alive more than once afterwards.
M="mysql -h0 -P9306 -N -B -e"
K=150

$M "DROP TABLE IF EXISTS t; CREATE TABLE t (article TEXT) rt_mem_limit='256M'"
vals=$(for i in $(seq 1 $K); do printf "(%d,'old')," $((1000000+i)); done)
$M "INSERT INTO t (id, article) VALUES ${vals%,}; FLUSH RAMCHUNK t"
D=$(dirname "$($M "FREEZE t" | grep -o '/[^ |]*t\.meta' | head -1)"); $M "UNFREEZE t" >/dev/null

manticore-load --quiet --batch-size=1000 --threads=1 --total=600000 --load="INSERT INTO t(id,article) VALUES(<increment>,'<text/50/100>')" >/dev/null 2>&1 &
if ! timeout 300 bash -c "until ls $D/t.1.* >/dev/null 2>&1; do :; done"; then echo "no disk chunk save seen"; pkill manticore-load; exit 1; fi

rm -f /tmp/issue-3501-frozen
( $M "FREEZE t" >/dev/null; touch /tmp/issue-3501-frozen ) &
for i in $(seq 1 $K); do
	[ -f /tmp/issue-3501-frozen ] && break
	( $M "REPLACE INTO t (id, article) VALUES ($((i+4)),'replaced'),($((1000000+i)),'replaced')" ) &
	sleep 0.02
done
until [ -f /tmp/issue-3501-frozen ]; do sleep 0.05; done
pkill manticore-load
sleep 0.5
$M "UNFREEZE t" >/dev/null
wait

echo "replaced ids alive more than once: $($M "SELECT id, count(*) c FROM t WHERE id<$((K+5)) GROUP BY id HAVING c>1 LIMIT 1000 OPTION max_matches=2000" | grep -c '[0-9]')"
