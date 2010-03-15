import os, time, sys, random, hashlib
from sphinxapi import *

if not sys.argv[1:]:
	print ( "Usage: python multyquery.py query_file [-i index -l N]\n" )
	sys.exit ( 1 )

sphinx = SphinxClient()
if not sphinx.Open():
	print ( "error: %s" % ( sphinx.GetLastError() ) )
	sys.exit ( 1 )

fp = open ( sys.argv[1], 'r' )
if not fp:
	print ( "failed to open %s", sys.argv[1:] )
	sys.exit ( 1 )

queries = fp.readlines()
fp.close()
i = 0
while i<len(queries):
	queries[i] = queries[i].rstrip('\n')
	i += 1

index = "*"
limit = 1000000
i = 0
while (i<len(sys.argv)):
	arg = sys.argv[i]
	if arg=='-i':
		index = sys.argv[i+1]
	elif arg=='-l':
		limit = int(sys.argv[i+1])
	i += 2

for q in queries:
	sphinx.SetSortMode ( SPH_SORT_EXTENDED, "@id asc" )
	sphinx.SetLimits ( 1, limit, limit )
	res = sphinx.Query ( q, index )

	if not res:
		print 'query failed: %s' % sphinx.GetLastError()
		sys.exit(1)

	if sphinx.GetLastWarning():
		print 'WARNING: %s\n' % sphinx.GetLastWarning()	
		
	ids = hashlib.md5()
	if res.has_key('matches'):
		for match in res['matches']:
			ids.update ( match['id'] )
	print 'query \'%-32s\', found %-6d, hash \'%s\'' % ( q, res['total_found'], ids.hexdigest() )
