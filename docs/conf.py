import os
import sys
import collections
try:
 if THEME_PATH:
	sys.path.append(THEME_PATH)
except NameError:
	pass
import sphinx_rtd_theme
sys.path.insert(0, os.path.abspath('..'))
sys.path.append(os.path.dirname(__file__))
sys.path.append(os.path.abspath('_ext'))

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.coverage',
    'sphinxprettysearchresults'
]
html_theme = "sphinx_rtd_theme"
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_theme_options = {
    'collapse_navigation': False,
    'display_version': True,
    'navigation_depth': 6,
}

master_doc = 'index'
source_suffix = '.rst'
project = u'Manticore Search'
copyright = u'2017, Manticore Search team'
version = '2.6.0'
release = version
exclude_patterns = ['_build', 'html', 'doctrees']
pygments_style = 'sphinx'
language = 'en'
man_pages = [
    ('index', 'manticoresearch', u'Manticore Search Documentation',
     [u'Manticore Search team'], 1)
]
latex_documents = [
  ('index', 'manticoresearch.tex', u'Manticore Search Documentation',
   u'The Manticore Search team', 'manual'),
]

texinfo_documents = [
  ('index', 'manticoresearch', u'Manticore Search Documentation',
   u'The Manticore Search team', 'Manticore Search','Open-source search server',
   'Search server'),
]
epub_title = u'Manticore Search'
epub_author = u'The Manticore Search team'
epub_publisher = u'The Manticore Search team'
epub_copyright = copyright
