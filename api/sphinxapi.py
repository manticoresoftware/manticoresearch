#
# $Id$
#

import select
import socket
from struct import *

SEARCHD_COMMAND_SEARCH	= 0
SEARCHD_COMMAND_EXCERPT	= 1

# current client-side command implementation versions
VER_COMMAND_SEARCH		= 0x104
VER_COMMAND_EXCERPT		= 0x100

# known searchd status codes
SEARCHD_OK				= 0
SEARCHD_ERROR			= 1
SEARCHD_RETRY			= 2

# known match modes
SPH_MATCH_ALL			= 0
SPH_MATCH_ANY			= 1
SPH_MATCH_PHRASE		= 2
SPH_MATCH_BOOLEAN		= 3

# known sort modes
SPH_SORT_RELEVANCE		= 0
SPH_SORT_ATTR_DESC		= 1
SPH_SORT_ATTR_ASC		= 2
SPH_SORT_TIME_SEGMENTS	= 3

# known attribute types
SPH_ATTR_INTEGER		= 1
SPH_ATTR_TIMESTAMP		= 2

# known grouping functions
SPH_GROUPBY_DAY	 		= 0
SPH_GROUPBY_WEEK		= 1
SPH_GROUPBY_MONTH		= 2
SPH_GROUPBY_YEAR		= 3
SPH_GROUPBY_ATTR		= 4

class SphinxClient:
	_host		= 'localhost'
	_port		= 3312
	_offset		= 0
	_limit		= 20
	_mode		= SPH_MATCH_ALL
	_weights	= []
	_sort		= SPH_SORT_RELEVANCE
	_sortby		= ''
	_min_id		= 0
	_max_id		= 0xFFFFFFFF
	_min		= {}
	_max		= {}
	_filter		= {}
	_groupby	= ''
	_groupfunc	= SPH_GROUPBY_DAY
	_maxmatches	= 1000

	_error		= ''
	_warning	= ''

	def __init__ (self):
		'''
		'''
		pass

	def GetLastError (self):
		'''
		'''
		return self._error

	def GetLastWarning (self):
		'''
		'''
		return self._warning

	def SetServer (self, host, port):
		'''
		'''
		assert(isinstance(host, str))
		assert(isinstance(port, int))

		self._host = host
		self._port = port

	def _Connect (self):
		'''
		'''
		try:
			sock = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
			sock.connect ( ( self._host, self._port ) )
		except socket.error, msg:
			if sock:
				sock.close()
			self._error = 'connection to %s:%s failed (%s)' % ( self._host, self._port, msg )
			return 0

		v = unpack('>L', sock.recv(4))
		if v<1:
			sock.close()
			self._error = 'expected searchd protocol version, got %s' % v
			return 0

		# all ok, send my version
		sock.send(pack('>L', 1))
		return sock

	def _GetResponse (self, sock, client_ver):
		'''
		'''
		(status, ver, length) = unpack('>2HL', sock.recv(8))
		response = ''
		left = length
		while left>0:
			chunk = sock.recv(left)
			if chunk:
				response += chunk
				left -= len(chunk)
			else:
				break

		sock.close()

		# check response
		read = len(response)
		if not response or read!=length:
			if length:
				self._error = 'failed to read searchd response (status=%s, ver=%s, len=%s, read=%s)' \
					% (status, ver, length, read)
			else:
				self._error = 'received zero-sized searchd response'
			return None

		# check status
		if status==SEARCHD_ERROR:
			self._error = 'searchd error: '+response[4:]
			return None

		if status==SEARCHD_RETRY:
			self._error = 'temporary searchd error: '+response[4:]
			return None

		if status!=SEARCHD_OK:
			self._error = 'unknown status code %d' % status
			return None

		# check version
		if ver<client_ver:
			self._warning = 'searchd command v.%d.%d older than client\'s v.%d.%d, some options might not work' \
				% (ver>>8, ver&0xff, client_ver>>8, client_ver&0xff)

		return response

	def SetLimits (self, offset, limit, maxmatches=0):
		'''
		'''
		assert(isinstance(offset, int) and offset>=0)
		assert(isinstance(limit, int) and limit>0)
		assert(maxmatches>=0)
		self._offset = offset
		self._limit = limit
		if maxmatches>0:
			self._maxmatches = maxmatches

	def SetMatchMode (self, mode):
		'''
		'''
		assert(mode in [SPH_MATCH_ALL, SPH_MATCH_ANY, SPH_MATCH_PHRASE, SPH_MATCH_BOOLEAN])
		self._mode = mode

	def SetSortMode (self, sort):
		'''
		'''
		assert(sort in [SPH_SORT_RELEVANCE, SPH_SORT_DATE_DESC, SPH_SORT_DATE_ASC, SPH_SORT_TIME_SEGMENTS])
		self._sort = sort
		
	def SetWeights (self, weights): 
		'''
		'''
		assert(isinstance(weights, list))
		for w in weights:
			assert(isinstance(w, int))
		self._weights = weights

	def SetIDRange (self, minid, maxid):
		'''
		'''
		assert(isinstance(minid, int))
		assert(isinstance(maxid, int))
		assert(minid<=maxid)
		self._min_id = minid
		self._max_id = maxid

	def SetFilter (self, attribute, values):
		'''
		'''
		assert(isinstance(attribute, str))
		assert(isinstance(values, list))
		assert(values)

		for value in values:
			assert(isinstance(value, int))

		self._filter[attribute] = values

	def SetFilterRange (self, attribute, min_, max_):
		'''

		'''
		assert(isinstance(attribute, str))
		assert(isinstance(min_, int))
		assert(isinstance(max_, int))
		assert(min_<=max_)

		self._min[attribute] = min_
		self._max[attribute] = max_

	def SetGroupBy (self, attribute, func):
		'''
		'''
		assert(isinstance(attribute, str))
		assert(func in [SPH_GROUPBY_DAY, SPH_GROUPBY_WEEK, SPH_GROUPBY_MONTH, SPH_GROUPBY_YEAR, SPH_GROUPBY_ATTR] )

		self._groupby = attribute
		self._groupfunc = func

	def Query (self, query, index='*'):
		'''
		'''
		sock = self._Connect()
		if not sock:
			return {}

		# build request
		req = [pack('>4L', self._offset, self._limit, self._mode, self._sort)]

		req.append(pack('>L', len(self._sortby)))
		req.append(self._sortby)

		req.append(pack('>L', len(query)))
		req.append(query)

		req.append(pack('>L', len(self._weights)))
		for w in self._weights:
			req.append(pack('>L', w))

		req.append(pack('>L', len(index)))
		req.append(index)
		req.append(pack('>L', self._min_id))
		req.append(pack('>L', self._max_id))

		# filters
		req.append(pack('>L', len(self._min)+len(self._filter)))

		for attr, m in self._min.iteritems():
			req.append(pack('>L', len(attr)))
			req.append(attr)
			req.append(pack('>3L', 0, m, self._max[attr]))

		for attr, values in self._filter.iteritems():
			req.append(pack('>L', len(attr)))
			req.append(attr)

			req.append(pack('>L', len(values)))
			for v in values:
				req.append(pack('>L', v))

		# group-by
		req.append(pack('>2L', self._groupfunc, len(self._groupby)))
		req.append(self._groupby)

		# max matches to retrieve
		req.append(pack('>L', self._maxmatches))

		# send query, get response
		req = ''.join(req)

		length = len(req)
		req = pack('>2HL', SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH, length)+req
		sock.send(req)
		response = self._GetResponse(sock, VER_COMMAND_SEARCH)
		if not response:
			return {}

		# parse response
		result = {}
		max_ = len(response)

		# read schema
		p = 0
		fields = []
		attrs = {}

		nfields = unpack('>L', response[p:p+4])[0]
		p += 4
		while nfields>0 and p<max_:
			nfields -= 1
			length = unpack('>L', response[p:p+4])[0]
			p += 4
			fields.append(response[p:p+length])
			p += length

		result['fields'] = fields

		nattrs = unpack('>L', response[p:p+4])[0]
		p += 4
		while nattrs>0 and p<max_:
			nattrs -= 1
			length = unpack('>L', response[p:p+4])[0]
			p += 4
			attr = response[p:p+length]
			p += length
			type_ = unpack('>L', response[p:p+4])[0]
			p += 4
			attrs[attr] = type_

		result['attrs'] = attrs

		# read match count
		count = unpack('>L', response[p:p+4])[0]
		p += 4

		# read matches
		while count>0 and p<max_:
			count -= 1
			doc, weight = unpack('>2L', response[p:p+8])
			p += 8

			result.setdefault('matches', {}).setdefault(doc, {}).setdefault('weight', weight)
			for attr, type_ in attrs.iteritems():
				val = unpack('>L', response[p:p+4])[0]
				p += 4
				result.setdefault('matches', {}).setdefault(doc, {}).setdefault('attrs', {}).setdefault(attr, val)

		result['total'], result['total_found'], result['time'], words = \
			unpack('>4L', response[p:p+16])

		result['time'] = '%.3f' % (result['time']/1000.0)
		p += 16

		while words>0:
			words -= 1
			length = unpack('>L', response[p:p+4])[0]
			p += 4
			word = response[p:p+length]
			p += length
			docs, hits = unpack('>2L', response[p:p+8])
			p += 8

			result.setdefault('words', {}).setdefault(word, {'docs':docs, 'hits':hits})

		sock.close()

		return result	

	def BuildExcerpts (self, docs, index, words, opts={}):
		'''
		'''
		assert(isinstance(docs, list))
		assert(isinstance(index, str))
		assert(isinstance(words, str))
		assert(isinstance(opts, dict))

		sock = self._Connect()

		if not sock:
			return {}

		# fixup options
		opts.setdefault('before_match', '<b>')
		opts.setdefault('after_match', '</b>')

		opts.setdefault('chunk_separator', ' ... ')
		opts.setdefault('limit', 256)
		opts.setdefault('around', 5)

		# build request
		# v.1.0 req

		# mode=0, flags=1 (remove spaces)
		req = [pack('>2L', 0, 1)]

		# req index
		req.append(pack('>L', len(index)))
		req.append(index)

		# req words
		req.append(pack('>L', len(words)))
		req.append(words)

		# options
		req.append(pack('>L', len(opts['before_match'])))
		req.append(opts['before_match'])

		req.append(pack('>L', len(opts['after_match'])))
		req.append(opts['after_match'])

		req.append(pack('>L', len(opts['chunk_separator'])))
		req.append(opts['chunk_separator'])

		req.append(pack('>L', int(opts['limit'])))
		req.append(pack('>L', int(opts['around'])))

		# documents
		req.append(pack('>L', len(docs)))
		for doc in docs:
			assert(isinstance(doc, str))
			req.append(pack('>L', len(doc)))
			req.append(doc)

		req = ''.join(req)

		# send query, get response
		length = len(req)

		# add header
		req = pack('>2HL', SEARCHD_COMMAND_EXCERPT, VER_COMMAND_EXCERPT, length)+req
		wrote = sock.send(req)

		response = self._GetResponse(sock, VER_COMMAND_EXCERPT )
		if not response:
			return {}

		# parse response
		pos = 0
		res = []
		rlen = len(response)

		for i in range(len(docs)):
			length = unpack('>L', response[pos:pos+4])[0]
			pos += 4

			if pos+length > rlen:
				self._error = 'incomplete reply'
				return []

			res.append(response[pos:pos+length])
			pos += length

		return res

#
# $Id$
#
