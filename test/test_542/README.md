# KNN rescore candidate regression

`test.xml` generates 2,000 deterministic, nonzero 16-dimensional vectors from SHA-256 coordinates. It tests cosine HNSW with `1bit` and without quantization. Each table is flushed once: RAM-only queries already use exact distances and would miss this bug. No external fixture is needed.

The 60 checks compare ID sets and row counts within each table layout, with and without explicit `max_matches`: a single RT table, two local shards, and both layouts accessed through loopback binary API agents. The single remote shard has 2,000 rows so candidate truncation at 1,000 is observable. `k=100,400,1000` covers `R=300,1200,3000`, including an explicit cap equal to `DEFAULT_MAX_MATCHES`. Additional checks cover result caps, OFFSET, and `rescore=0`. Local and agent results must also agree for these single-chunk shards.

`model.bin` contains the expected boolean invariants in UberTest's serialized custom-test format. It does not contain daemon-generated reference IDs. `test/CMakeLists.txt` discovers the new `test_542` directory automatically; no registration change is needed. Do not regenerate the model to accept a failure.

Changed source files: `src/queuecreator.cpp`, `src/knnmisc.cpp`, `src/knnmisc.h`. Added files: `test/test_542/test.xml`, `test/test_542/model.bin`, `test/test_542/manual.sql`, `test/test_542/README.md`.

## Semantics and remaining boundary

For KNN queries that perform exact rescoring, explicit `max_matches` caps the sorted results, including the rows skipped by OFFSET. It does not cap the approximate candidates: the shard queue retains at least `GetRequestedDocs()` (`floor(k*oversampling)`) before rescoring. Trimming happens after exact sorting. Merge queues consume the shards' exact distances and retain the explicit result cap. Implicit `max_matches` and `rescore=0` retain their existing behavior.

Agent candidate capacity derives from the KNN settings even though the binary API still infers the explicit flag from `max_matches != DEFAULT_MAX_MATCHES`. No protocol fields or command versions change.

Agent rescore-window harmonization is deferred. Agents still receive `LIMIT=max_matches`, while a local shard uses the original LIMIT+OFFSET. With several disk chunks, an agent may therefore rescore more candidates. Changing this would alter uncapped query results and require separate review of forwarded limits, outer ordering, and divided limits. This test uses one disk chunk per shard and does not claim to resolve that difference.

## Build and run on dev2

These commands document the operator workflow; none were executed during PR preparation. The existing sync script mirrors this worktree, including its pre-existing MCL changes. The build script builds `searchd`, `indexer`, `indextool`, `knn_lib`, `columnar_lib`, and `secondary_index`, then bakes `kbd:dev`.

On the Mac:

```sh
cd /Users/dk/Work/dev/manticore/knn-build-fix
./kbd-tools/sync.sh
ssh dmitrii@dev2.manticoresearch.com 'cd ~/kbd && ./tools/build.sh'
```

In a dev2 shell, run the new test using the built binaries and modules. The repository's CI UberTest image supplies PHP and MySQL; it must already be cached. This command cannot pull an image and gives the test container only loopback networking. MySQL is required by the existing custom-test harness even though this test has no SQL data source.

```sh
cd ~/kbd
docker image inspect manticoresearch/ubertests_public:332_aot >/dev/null
mkdir -p "$HOME/kbd/build/knn-rescore-test"
docker run --rm --pull=never --network=none \
  -v "$HOME/kbd/manticoresearch:/repo:ro" \
  -v "$HOME/kbd/build/img:/bins:ro" \
  -v "$HOME/kbd/build/knn-rescore-test:/results" \
  -e MANTICORE_MODULES=/bins -e FULL_SHARE_DIR=/repo/misc \
  -e MANTICORE_NO_BUDDY=1 -w /repo/test \
  manticoresearch/ubertests_public:332_aot \
  bash -euo pipefail -c '
    mysqladmin --wait=60 --silent ping
    exec php ubertest.php --ctest --strict-verbose --no-demo --rt \
      -u test -b /bins/ -t /results/ -tt /results/ t 542
  '
```

The supplied dev2 logs confirm `ALL OK`, `EXIT=0`, and zero skips on the fixed build (7.06 seconds). The pre-fix run against image `sha256:bf6be4b043a864f720ee43c97bf823a1c8f824468f4b180bb5413a6fbb485aca` failed subtest 1 with `EXIT=1` and zero skips (5.08 seconds): 17 of the 60 boolean checks were false, all for `1bit rescore_*` cases. These cover all four layouts at `k=100` and `k=400`, the full table at `k=1000`, and result/offset caps in all four layouts. All unquantized checks, local/agent and direct/remote comparisons, and `rescore=0` checks were true. The pre-fix read-only-filesystem warnings concern diagnostic diff files and do not cause the recorded boolean mismatches.

The runs used `run542.sh` and `run542_old.sh` with the CI runner image above. The negative-control script extracts modules from the old image; identical module binaries across the two runs were not established. The exact mismatch list, scripts, and log provenance accompany the PR text. Repeat with `t 298 515 517 541 542` to check neighboring quantization, result-window, distributed, and hybrid cases; those broader runs remain unverified here.

## Plain SQL manual recall check

`manual.sql` creates a separate 64-row table and runs three searches: exact fullscan ground truth, HNSW+rescore without a cap, and the same HNSW query with `max_matches=10`. Compare the two HNSW ID sets; they must match after the fix. Recall@10 for either is the number of IDs shared with the fullscan result divided by 10. Equal HNSW sets do not require perfect HNSW recall.

Start an isolated container on dev2:

```sh
docker run -d --rm --pull=never --network=none --name knn-rescore-manual \
  -v "$HOME/kbd/manticoresearch/test/test_542:/fixture:ro" kbd:dev
docker exec knn-rescore-manual mysqladmin --protocol=tcp -h127.0.0.1 -P9306 --wait=60 --silent ping
docker exec knn-rescore-manual sh -ec 'mysql --protocol=tcp -h127.0.0.1 -P9306 < /fixture/manual.sql'
docker stop knn-rescore-manual
```

The SQL can also be pasted into any running `kbd:dev` container's MySQL client. To check the unquantized path manually, repeat it with the `quantization='1bit'` clause removed from CREATE TABLE.

## Local validation

XML parsing, extracted PHP syntax, deterministic fixture shape, serialized model structure and loading through the existing UberTest helper, shell command syntax, and `git diff --check` were checked locally. No daemon compilation, integration test, container execution, network request, or remote command was performed on the Mac. `clang-format` is unavailable locally; changed C++ follows the surrounding style.
