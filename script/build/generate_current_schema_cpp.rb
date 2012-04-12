#!/usr/bin/env ruby

require 'pathname'

# Project base directory (this script is assumed to be in script/build)
base_dir=Pathname.new(File.dirname(__FILE__)).join("..", "..")

# The template reads a file and assumes we're in the project base directory
Dir.chdir(base_dir)

# File names
template=Pathname.pwd.join('src', 'db', 'schema', 'CurrentSchema.cpp.erb')
output  =Pathname.pwd.join('build', 'CurrentSchema.cpp')

puts "Writing #{output}"

output.dirname.mkpath
if !system("erb -T 1 #{template} >#{output}")
	# Generation failed. Delete the output so we don't leave an old version behind
	puts "Generation of #{output} failed"
	output.delete
end
