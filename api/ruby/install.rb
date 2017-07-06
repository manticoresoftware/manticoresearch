require 'fileutils'

sphinx_config = File.dirname(__FILE__) + '/../../../config/sphinx.yml'
FileUtils.cp File.dirname(__FILE__) + '/sphinx.yml.tpl', sphinx_config unless File.exist?(sphinx_config)
puts IO.read(File.join(File.dirname(__FILE__), 'README'))
