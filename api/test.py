#
# $Id$
#

from sphinxapi import *
import sys, time

if not sys.argv[1:]:
	print 'usage: test.py [--any] <word [word [word [...]]]> [--group <group>] [-p <port>] [-i <index>]\n'
	sys.exit(0)

q = ''
mode = SPH_MATCH_ALL
groups = []
port = 3312
index = '*'

i = 1
while (i<len(sys.argv)):
	arg = sys.argv[i]
	if arg=='-a' or arg=='--any':
		mode = SPH_MATCH_ANY
	elif arg=='-b' or arg=='--boolean':
		mode = SPH_MATCH_BOOLEAN
	elif arg=='-e' or arg=='--extended':
		mode = SPH_MATCH_EXTENDED
	elif arg=='-g' or arg=='--group':
		groups.append ( int(sys.argv[++i]) )
	elif arg=='-p' or arg=='--port':
		port = int(sys.argv[++i])
	elif arg=='-i':
		i += 1
		index = sys.argv[++i]
	else:
		q = '%s%s ' % ( q, arg )
	i += 1

# do query
cl = SphinxClient()
cl.SetServer('localhost', port)
cl.SetWeights([100, 1])
cl.SetMatchMode ( mode )

if groups:
	cl.SetFilter('group_id', groups)

res = cl.Query(q, index)

if not res:
	print 'query failed: %s' % cl.GetLastError()
	sys.exit(1)

if cl.GetLastWarning():
	print 'WARNING: %s\n' % cl.GetLastWarning()

print 'Query \'%s\' retrieved %d of %d matches in %s sec' % (q, res['total'], res['total_found'], res['time'])
print 'Query stats:'

if res.has_key('words'):
	for info in res['words']:
		print '\t\'%s\' found %d times in %d documents' % (info['word'], info['hits'], info['docs'])

if res.has_key('matches'):
	n = 1
	print '\nMatches:'
	for match in res['matches']:
		attrsdump = ''
		for attr in res['attrs']:
			attrname = attr[0]
			attrtype = attr[1]
			value = match['attrs'][attrname]
			if attrtype==SPH_ATTR_TIMESTAMP:
				value = time.strftime ( '%Y-%m-%d %H:%M:%S', time.localtime(value) )
			attrsdump = '%s, %s=%s' % ( attrsdump, attrname, value )

		print '%d. doc_id=%s, weight=%d%s' % (n, match['id'], match['weight'], attrsdump)
		n += 1

#
# $Id$
#
