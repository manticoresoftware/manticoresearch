#
# $Id$
#

from sphinxapi import *
import sys, time

if not sys.argv[1:]:
	print 'usage: test.py [--any] <word [word [word [...]]]> [--group <group>] [-p <port>] [-i <index>]\n'
	sys.exit(0)

q = ''
any = False
groups = []
port = 3312
index = '*'

i = 1
while (i<len(sys.argv)):
	if sys.argv[i] == '--any':
		any = True
	elif i == '--group':
		i += 1
		groups = sys.argv[i]
	elif sys.argv[i] == '-p':
		i += 1
		port = sys.argv[i]
	elif sys.argv[i] == '-i':
		i += 1
		index = sys.argv[i]
	else:
		q = '%s%s ' % (q, sys.argv[i])
	i += 1

# do query
cl = SphinxClient()
cl.SetServer('localhost', port)
cl.SetWeights([100, 1])

if any:
	cl.SetMatchMode(SPH_MATCH_ANY)
else:
	cl.SetMatchMode(SPH_MATCH_ALL)

if groups:
	cl.SetFilter('group_id', groups)

res = cl.Query(q, index)

if not res:
	print 'query failed: %s' % cl.GetLastError()
	sys.exit(1)

if cl.GetLastWarning():
	print 'WARNING: %s' % cl.GetLastWarning()

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
