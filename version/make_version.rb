#!/usr/bin/env ruby

require 'date'

def svn_version
	if File.directory?('.svn')
		# This should be portable, so we can't use 'which' or redirect stderr
		# (both won't work on Windows).
		if File.exist?('/usr/bin/svnversion')
			svn_version=`svnversion ..`.chomp
			"-#{svn_version}"
		else
			"-svn"
		end
	else
		""
	end
end

version=IO.readlines("version")[0].chomp
date=Date.today.to_s
svn=svn_version

version_string="#{version}#{svn} (#{date})"
puts "Version is #{version_string}"

File.open("version.h", "w") { |file|
	file.puts "#define VERSION \"Version #{version_string}\""
}

