#!/usr/bin/env ruby

require 'pathname'
require 'date'

# Project base directory (this script is assumed to be in script/build)
base_dir=Pathname.new(File.dirname(__FILE__)).join("..", "..")

# File names
version_file=base_dir.join('version', 'version')
output      =base_dir.join('build', 'version.h')

def svn_version(dir)
  if File.directory?('.svn')
		# This should be portable, so we can't use 'which' or redirect stderr
		# (both won't work on Windows).
		if File.exist?('/usr/bin/svnversion')
			svn_version=`svnversion #{dir}`.chomp
			"-#{svn_version}"
		else
			"-svn"
		end
	else
		""
	end
end

version=IO.readlines(version_file)[0].chomp
date=Date.today.to_s
svn=svn_version(base_dir)

version_string="#{version}#{svn} (#{date})"

puts "Writing #{output}"
puts "Version is #{version_string}"

output.dirname.mkpath
File.open(output, "w") { |file|
	file.puts "#define VERSION \"Version #{version_string}\""
}
