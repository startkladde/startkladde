#!/usr/bin/env ruby

require 'pathname'

### Change to the project base directory 

# This script is assumed to be located in script/build
Dir.chdir Pathname.new(File.dirname(__FILE__)).join("..", "..")


### Generate the version

# Call the make_version script in the version directory
Dir.chdir("version") {
	system "ruby make_version.rb"
}

### Generate the current schema
 
template=Pathname.pwd.join('src', 'db', 'schema', 'CurrentSchema.cpp.erb')
output  =Pathname.pwd.join('build', 'CurrentSchema.cpp')

# Make sure the directory for the output exists
output.dirname.mkpath
if !system("erb -T 1 #{template} >#{output}")
	# Generation failed. Delete the output so we don't leave an old version behind
	puts "Generation of #{output} failed"
	output.delete
end
