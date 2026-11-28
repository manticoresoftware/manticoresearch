# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

"""Exercise long hitlists across documents, flushes and merges."""

import argparse
import json
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--url', required=True)
    parser.add_argument('--access', choices=['mmap', 'file'], default='mmap')
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    endpoint = args.url.rstrip('/') + '/sql?mode=raw'

    def sql(query):
        req = urllib.request.Request(endpoint, data=urllib.parse.urlencode({'query': query}).encode())
        try:
            result = json.loads(urllib.request.urlopen(req, timeout=120).read())
        except urllib.error.HTTPError as error:
            raise AssertionError((query[:300], error.read().decode())) from error
        assert all(not x.get('error') and not x.get('warning') for x in result), (query, result)
        return result[0].get('data', [])

    tables = ['longhits_' + args.access + '_' + fmt for fmt in ['legacy', 'block']]
    for table, fmt in zip(tables, ['legacy', 'block']):
        sql("DROP TABLE IF EXISTS " + table)
        sql(f"CREATE TABLE {table} (title text, body text) postings_format='{fmt}' index_field_lengths='1' min_prefix_len='2' access_doclists='{args.access}' access_hitlists='{args.access}'")

    matches = ['"alpha beta"', '"alpha beta gamma"', '"alpha alpha"',
               'alpha NEAR/3 gamma', '"alpha beta"~4', 'alpha beta',
               'alpha | beta | gamma', '("alpha beta" | gamma) !absent',
               '@body "alpha beta"', '"alp* beta"', '"alpha beta$"']
    checks = []

    def compare(stage):
        for match in matches:
            for ranker in ['none', 'bm25', 'proximity_bm25']:
                query = "SELECT id,weight() FROM {} WHERE MATCH('" + match + "') ORDER BY id ASC LIMIT 1000 OPTION max_matches=1000,threads=1,ranker=" + ranker
                rows = [sql(query.format(table)) for table in tables]
                assert rows[0] == rows[1], (stage, match, ranker, rows)
                checks.append(dict(stage=stage, match=match, ranker=ranker, rows=rows[0]))

    for chunk in range(2):
        for start in range(0, 270, 30):
            values = []
            for i in range(start, start + 30):
                docid = chunk * 270 + i + 1
                # Mix long and short hitlists to exercise buffered-reader boundaries.
                count = 4101 if i % 53 == 0 else (i % 9 + 1)
                text = ('alpha beta gamma ' * count) + ('alpha alpha' if i % 2 else 'alpha beta')
                values.append(f"({docid},'beta alpha','{text}')")
            for table in tables:
                sql('INSERT INTO ' + table + ' VALUES ' + ','.join(values))
        compare('ram' if chunk == 0 else 'mixed')
        for table in tables:
            sql('FLUSH RAMCHUNK ' + table)
        compare('disk' + str(chunk))

    for table in tables:
        sql('DELETE FROM ' + table + ' WHERE id IN (1,54,107,271,324)')
        sql('OPTIMIZE TABLE ' + table + ' OPTION sync=1')
    compare('merged_deleted')
    Path(args.output).write_text(json.dumps(dict(access=args.access, checks=checks), indent=2))
    print(len(checks), 'long-hit comparisons passed')


if __name__ == '__main__':
    main()
