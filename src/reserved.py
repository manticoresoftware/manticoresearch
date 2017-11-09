# autocheck that reserved keywords match across lexer/parser/sources/docs

import re

def die(m):
	print m
	exit(1)

def load(n):
	f = open(n)
	v = f.read()
	f.close()
	return v

def diff(a, b):
	b = set(b)
	return [aa for aa in a if aa not in b]

# load special tokens from lexer
res = []
f = open('sphinxql.l')
for line in f:
	r = re.match('^"(\w+)"\s+', line)
	if r:
		res.append(r.group(1))
f.close()
if not res:
	die('failed to extract resreved keywords from src/sphinxql.l')

# remove those that are handled by parser
r = re.search('ident_set:(.*?);', load('sphinxql.y'), re.MULTILINE + re.DOTALL)
if not r:
	die('failed to extract ident_set from src/sphinxql.y')
handled = [k[4:] for k in re.findall('\w+', r.group(1)) if k!='TOK_IDENT' and k[0:4]=='TOK_']
handled.append('NAMES') # manually append a couple tweaks
handled.append('TRANSACTION')
res = sorted(diff(res, handled))

# load reserved keywords list from docs
r = re.search('\.\. code\-block:: mysql(.*)', load('../docs/sphinxql_reference/list_of_sphinxql_reserved_keywords.rst'), re.MULTILINE + re.DOTALL)
if not r:
	die('failed to extract reserved keywords from docs/sphinxql_reference/list_of_sphinxql_reserved_keywords.rst')
doc = [k for k in re.findall('(\w+)', r.group(1))]

# load reserved keywords list from sources
r = re.search('dReserved\[\]\s+=\s+\{(.*?)\}', load('sphinx.cpp'), re.MULTILINE + re.DOTALL)
if not r:
	die('failed to extract reserved keywords from src/sphinx.cpp')
src = [k for k in re.findall('"(\w+)"', r.group(1))]


# now report
not_in_docs = sorted(diff(res, doc))
if not_in_docs:
	print '=== reserved but not in docs: ' + ', '.join(not_in_docs) + '\n'
	print '.. code-block:: mysql'
	s = ''
	for k in res:
		if len(s) + len(k) >= 60:
			print s.strip()
			s = ''
		s += k + ', '
	if s:
		print s.strip()[:-1]
	print '\n'

not_in_src = sorted(diff(res, src))
if not_in_src:
	print '=== reserved but not in sources: ' + ', '.join(not_in_src) + '\n'
	print '\tstatic const char * dReserved[] =\n\t{'
	s = ''
	for k in res:
		if len(s) + len(k) >= 80:
			print '\t\t' + s.strip()
			s = ''
		s += '"%s", ' % k
	print '\t\t' + s + 'NULL\n\t};\n'

docs_not_res = sorted(diff(doc, res))
if docs_not_res:
	print '=== in docs but not reserved: ' + ', '.join(docs_not_res) + '\n'

src_not_res = sorted(diff(src, res))
if src_not_res:
	print '=== in sources but not reserved: ' + ', '.join(src_not_res) + '\n'

if not_in_docs or not_in_src or docs_not_res or src_not_res:
	die('got errors')
