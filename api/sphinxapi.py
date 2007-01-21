#
# $Id$
#
# Python version of Sphinx searchd client (Python API)
#
# Copyright (c) 2006, Andrew Aksyonoff
# Copyright (c) 2006, Mike Osadnik
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/
#

import select
import socket
from struct import *


# known searchd commands
SEARCHD_COMMAND_SEARCH	= 0
SEARCHD_COMMAND_EXCERPT	= 1

# current client-side command implementation versions
VER_COMMAND_SEARCH		= 0x106
VER_COMMAND_EXCERPT		= 0x100

# known searchd status codes
SEARCHD_OK				= 0
SEARCHD_ERROR			= 1
SEARCHD_RETRY			= 2
SEARCHD_WARNING			= 3

# known match modes
SPH_MATCH_ALL			= 0
SPH_MATCH_ANY			= 1
SPH_MATCH_PHRASE		= 2
SPH_MATCH_BOOLEAN		= 3
SPH_MATCH_EXTENDED		= 4

# known sort modes
SPH_SORT_RELEVANCE		= 0
SPH_SORT_ATTR_DESC		= 1
SPH_SORT_ATTR_ASC		= 2
SPH_SORT_TIME_SEGMENTS	= 3
SPH_SORT_EXTENDED		= 4

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
	_host		= 'localhost'			# searchd host (default is "localhost")
	_port		= 3312					# searchd port (default is 3312)
	_offset		= 0						# how much records to seek from result-set start (default is 0)
	_limit		= 20					# how much records to return from result-set starting at offset (default is 20)
	_mode		= SPH_MATCH_ALL			# query matching mode (default is SPH_MATCH_ALL)
	_weights	= []					# per-field weights (default is 1 for all fields)
	_sort		= SPH_SORT_RELEVANCE	# match sorting mode (default is SPH_SORT_RELEVANCE)
	_sortby		= ''					# attribute to sort by (defualt is "")
	_min_id		= 0						# min ID to match (default is 0)
	_max_id		= 0xFFFFFFFF			# max ID to match (default is UINT_MAX)
	_filters	= []					# search filters
	_groupby	= ''					# group-by attribute name
	_groupfunc	= SPH_GROUPBY_DAY		# function to pre-process group-by attribute value with
	_sortbygroup= 1						# whether to sort grouped results by group, or by current sorting func
	_maxmatches	= 1000					# max matches to retrieve

	_error		= ''					# last error message
	_warning	= ''					# last warning message


	def __init__ (self):
		"""
		create a new client object and fill defaults
		"""
		pass


	def GetLastError (self):
		"""
		get last error message (string)
		"""
		return self._error


	def GetLastWarning (self):
		"""
		get last warning message (string)
		"""
		return self._warning


	def SetServer (self, host, port):
		"""
		set searchd server
		"""
		assert(isinstance(host, str))
		assert(isinstance(port, int))

		self._host = host
		self._port = port


	def _Connect (self):
		"""
		connect to searchd server
		"""
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
		"""
		get and check response packet from searchd server
		"""
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
		if status==SEARCHD_WARNING:
			wend = 4 + unpack ( '>L', response[0:4] )[0]
			self._warning = response[4:wend]
			return response[wend:]

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
		"""
		set match offset, count, and max number to retrieve
		"""
		assert(isinstance(offset, int) and offset>=0)
		assert(isinstance(limit, int) and limit>0)
		assert(maxmatches>=0)
		self._offset = offset
		self._limit = limit
		if maxmatches>0:
			self._maxmatches = maxmatches


	def SetMatchMode (self, mode):
		"""
		set match mode
		"""
		assert(mode in [SPH_MATCH_ALL, SPH_MATCH_ANY, SPH_MATCH_PHRASE, SPH_MATCH_BOOLEAN, SPH_MATCH_EXTENDED])
		self._mode = mode


	def SetSortMode (self, sort):
		"""
		set sort mode
		"""
		assert(sort in [SPH_SORT_RELEVANCE, SPH_SORT_DATE_DESC, SPH_SORT_DATE_ASC, SPH_SORT_TIME_SEGMENTS, SPH_SORT_EXTENDED])
		self._sort = sort


	def SetWeights (self, weights): 
		"""
		set per-field weights
		"""
		assert(isinstance(weights, list))
		for w in weights:
			assert(isinstance(w, int))
		self._weights = weights


	def SetIDRange (self, minid, maxid):
		"""
		set IDs range to match
		only match those records where document ID
		is beetwen minid and maxid (including minid and maxid)
		"""
		assert(isinstance(minid, int))
		assert(isinstance(maxid, int))
		assert(minid<=maxid)
		self._min_id = minid
		self._max_id = maxid


	def SetFilter ( self, attribute, values, exclude=0 ):
		"""
		set values filter
		only match those records where $attribute column values
		are in specified set
		"""
		assert(isinstance(attribute, str))
		assert(isinstance(values, list))
		assert(values)

		for value in values:
			assert(isinstance(value, int))

		self._filters.append ( { 'attr':attribute, 'exclude':exclude, 'values':values } )


	def SetFilterRange (self, attribute, min_, max_, exclude=0 ):
		"""
		set range filter
		only match those records where $attribute column value
		is beetwen $min and $max (including $min and $max)
		"""
		assert(isinstance(attribute, str))
		assert(isinstance(min_, int))
		assert(isinstance(max_, int))
		assert(min_<=max_)

		self._filters.append ( { 'attr':attribute, 'exclude':exclude, 'min':min_, 'max':max_ } )


	def SetGroupBy ( self, attribute, func, sortbygroup=1 ):
		"""
		set grouping attribute and function

		in grouping mode, all matches are assigned to different groups
		based on grouping function value.

		each group keeps track of the total match count, and the best match
		(in this group) according to current sorting function.

		the final result set contains one best match per group, with
		grouping function value and matches count attached.

		result set could be sorted either by 1) grouping function value
		in descending order (this is the default mode, when $sortbygroup
		is set to true); or by 2) current sorting function (when $sortbygroup
		is false).

		WARNING, when sorting by current function there might be less
		matching groups reported than actually present. @count might also be
		underestimated. 

		for example, if sorting by relevance and grouping by "published"
		attribute with SPH_GROUPBY_DAY function, then the result set will
		contain one most relevant match per each day when there were any
		matches published, with day number and per-day match count attached,
		and sorted by day number in descending order (ie. recent days first).
		"""
		assert(isinstance(attribute, str))
		assert(func in [SPH_GROUPBY_DAY, SPH_GROUPBY_WEEK, SPH_GROUPBY_MONTH, SPH_GROUPBY_YEAR, SPH_GROUPBY_ATTR] )

		self._groupby = attribute
		self._groupfunc = func
		self._sortbygroup = sortbygroup


	def Query (self, query, index='*'):
		"""
		connect to searchd server and run given search query

		"query" is query string
		"index" is index name to query, default is "*" which means to query all indexes

		returns false on failure
		returns hash which has the following keys on success:
			"matches"
				an array of found matches represented as ( "id", "weight", "attrs" ) hashes
			"total"
				total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
			"total_found"
				total amount of matching documents in index
			"time"
				search time
			"words"
				an array of ( "word", "docs", "hits" ) hashes which contains
				docs and hits count for stemmed (!) query words
		"""
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
		req.append ( pack ( '>L', len(self._filters) ) )
		for f in self._filters:
			req.append ( pack ( '>L', len(f['attr']) ) )
			req.append ( f['attr'] )
			if ( 'values' in f ):
				req.append ( pack ( '>L', len(f['values']) ) )
				for v in f['values']:
					req.append ( pack ( '>L', v ) )
			else:
				req.append ( pack ( '>3L', 0, f['min'], f['max'] ) )
			req.append ( pack ( '>L', f['exclude'] ) )

		# group-by
		req.append(pack('>2L', self._groupfunc, len(self._groupby)))
		req.append(self._groupby)

		# max matches, sort-by-group
		req.append(pack('>2L', self._maxmatches, self._sortbygroup))

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
		attrs = []

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
			attrs.append([attr,type_])

		result['attrs'] = attrs

		# read match count
		count = unpack('>L', response[p:p+4])[0]
		p += 4

		# read matches
		result['matches'] = []
		while count>0 and p<max_:
			count -= 1
			doc, weight = unpack('>2L', response[p:p+8])
			p += 8

			match = { 'id':doc, 'weight':weight, 'attrs':{} }
			for i in range(len(attrs)):
				match['attrs'][attrs[i][0]] = unpack('>L', response[p:p+4])[0]
				p += 4

			result['matches'].append ( match )

		result['total'], result['total_found'], result['time'], words = \
			unpack('>4L', response[p:p+16])

		result['time'] = '%.3f' % (result['time']/1000.0)
		p += 16

		result['words'] = []
		while words>0:
			words -= 1
			length = unpack('>L', response[p:p+4])[0]
			p += 4
			word = response[p:p+length]
			p += length
			docs, hits = unpack('>2L', response[p:p+8])
			p += 8

			result['words'].append({'word':word, 'docs':docs, 'hits':hits})

		sock.close()

		return result	


	def BuildExcerpts (self, docs, index, words, opts=None):
		"""
		connect to searchd server and generate exceprts from given documents

		"docs" is an array of strings which represent the documents' contents
		"index" is a string specifiying the index which settings will be used
			for stemming, lexing and case folding
		"words" is a string which contains the words to highlight
		"opts" is a hash which contains additional optional highlighting parameters:
			"before_match"
				a string to insert before a set of matching words, default is "<b>"
			"after_match"
				a string to insert after a set of matching words, default is "<b>"
			"chunk_separator"
				a string to insert between excerpts chunks, default is " ... "
			"limit"
				max excerpt size in symbols (codepoints), default is 256
			"around"
				how much words to highlight around each match, default is 5

		returns false on failure
		returns an array of string excerpts on success
		"""
		if not opts:
			opts = {}

		assert(isinstance(docs, list))
		assert(isinstance(index, str))
		assert(isinstance(words, str))
		assert(isinstance(opts, dict))

		sock = self._Connect()

		if not sock:
			return []

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
			return []

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
