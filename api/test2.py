#
# $Id$
#
from __future__ import print_function
from sphinxapi import *
import sys

docs = ['this is my test text to be highlighted','this is another test text to be highlighted']
words = 'test text'
index = 'forum'

opts = {'before_match':'<b>', 'after_match':'</b>', 'chunk_separator':' ... ', 'limit':400, 'around':15}

cl = SphinxClient()
res = cl.BuildExcerpts(docs, index, words, opts)

if not res:
	print  ('ERROR: %s\n' % cl.GetLastError())
else:
	n = 0
	for entry in res:
		n += 1
		print ('n=%d, res=%s' % (n, entry))

#
# $Id$
#
