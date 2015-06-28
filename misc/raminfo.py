# a simple tool to examine sphinx rt index .ram file

import sys, struct, os


def get_dword(f):
	return struct.unpack('I', f.read(4))[0]


def get_id(f, id64):
	if id64:
		return struct.unpack('Q', f.read(8))[0]
	else:
		return struct.unpack('I', f.read(4))[0]


def skip_vec(f, entry_size):
	f.seek(get_dword(f) * entry_size, 1)


def main(argv):
	if len(argv) < 2:
		sys.stderr.write("usage: raminfo.py <filename.ram>")
		return 1

	if not os.path.exists(argv[1]):
		sys.stderr.write("ERROR: file %r not found!" % argv[1])
		return 1

	f = open(argv[1], "rb")
	print "examining RAM chunk %r..." % argv[1]
	id64 = get_dword(f)
	if id64!=0:
		id64 = 1
	next_tag = get_dword(f)
	num_segments = get_dword(f)
	print "%d segments total, %d-bit ids, next free tag %d" % (num_segments, 32*(1+id64), next_tag)

	if num_segments>32:
		print "WARNING, unexpectedly many segments, displaying first 32 only"

	index_minid = 0
	index_maxid = 0
	total_rows = 0
	total_alive = 0

	for i in range(1, num_segments+1):
		off1 = f.tell()
		tag = get_dword(f)
		skip_vec(f, 1) # keywords
		skip_vec(f, 1) # checkpoints1
		skip_vec(f, 16) # checkpoints2
		skip_vec(f, 1) # docs
		skip_vec(f, 1) # hits

		rows = get_dword(f)
		alive = get_dword(f)
		off2 = f.tell()
		skip_vec(f, 4) # rows data
		off3 = f.tell()

		rowsize = (off3 - off2 - 4)/rows
		f.seek(off2 + 4)
		minid = get_id(f, id64)
		f.seek(off3 - rowsize)
		maxid = get_id(f, id64)
		f.seek(off3)

		skip_vec(f, 4*(1+id64)) # klist
		skip_vec(f, 1) # strings
		skip_vec(f, 4) # mva
		skip_vec(f, 8) # infixcp

		xrows = ""
		if rows!=alive:
			xrows = " (alive %d)" % alive

		if i<=32:
			print "seg %d at off %d, tag %d, rows %d%s at off %d, %d b/row, ids %d..%d" % (i, off1, tag, rows, xrows, off2, rowsize, minid, maxid)
		else:
			if sys.stdout.isatty() and (i % 1000)==0:
				sys.stdout.write("scanning segment %d of %d\r" % (i, num_segments))

		if index_maxid==0:
			index_minid = minid
			index_maxid = maxid
		else:
			index_minid = min(index_minid, minid)
			index_maxid = max(index_maxid, maxid)
		total_rows += rows
		total_alive += alive

	print "total %d rows (%d alive), ids %d..%d" % (total_rows, total_alive, index_minid, index_maxid)

if __name__ == "__main__":
	sys.exit(main(sys.argv))
