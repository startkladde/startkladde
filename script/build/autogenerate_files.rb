#!/usr/bin/env ruby

require 'pathname'

dir=Pathname.new(File.dirname(__FILE__))

# Use expand_path so we get an absolute path, because on Unix systems, the
# current directory is not included in the search path
system dir.join('generate_version_h.rb').expand_path
system dir.join('generate_current_schema_cpp.rb').expand_path
