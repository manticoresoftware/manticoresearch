#
# $Id$
#

import sys, re

if len(sys.argv)!=3:
	print 'Usage: python resolve.py BACKTRACE SYMBOLS'
	sys.exit(0)

def myopen(name):
	if name == '-':
		return sys.stdin
	fh = open(name, 'r')
	if not fh:
		print 'FATAL: failed to open %s' % name
		sys.exit(1)
	return fh

syms = []
fp = myopen(sys.argv[2])
for line in fp.readlines():
	line = line.rstrip()
	match = re.match('([0-9a-fA-F]+) \w ', line)
	if match:
		addr = int(match.group(1), 16)
		name = line[len(match.group(0)):]
		syms.append([addr, name])
fp.close()

fp = myopen(sys.argv[1])
for line in fp.readlines():
	line = line.rstrip()
	match = re.search('\[0x([0-9a-fA-F]+)\]', line)
	if match:
		addr = int(match.group(1), 16)
		resolved = '???'
		for i in range(len(syms)-1):
			if syms[i][0]<=addr and addr<syms[i+1][0]:
				resolved = '%s+0x%x' % (syms[i][1], addr - syms[i][0])
				break
		line += ' ' + resolved
	print line
fp.close()
