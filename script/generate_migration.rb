#!/usr/bin/env ruby

require 'erb'

force=false
name=""

ARGV.each do |arg|
	case arg
	when "-f", "--force" then force=true
	when /^-/            then puts "Unknown argument: #{arg}"; exit 1
	else name=arg
	end
end

if name.empty?
	puts "Usage: [-f|--force] #{$0} name"
	puts "  name: the name of the migration, without the version"
	puts "Example: #{$0} add_towpilot"
	exit 1
end

timestamp=Time.now.utc.strftime "%Y%m%d%H%M%S"

dir="src/db/migrations"

source_template="#{dir}/Migration.cpp.erb"
header_template="#{dir}/Migration.h.erb"

full_name="#{timestamp}_#{name}"
class_name="Migration_#{full_name}"

source="#{dir}/Migration_#{timestamp}_#{name}.cpp"
header="#{dir}/Migration_#{timestamp}_#{name}.h"

if !File.directory?(dir)
	puts "Directory #{dir} does not exist - this script must be called from the root"
	exit 1
end

(puts "Template #{source_template} does not exist"; exit 1) if !File.file?(source_template)
(puts "Template #{header_template} does not exist"; exit 1) if !File.file?(header_template)

(puts "#{source} exists - specify --force to overwrite"; exit 1) if (File.exist?(source) && !force)
(puts "#{header} exists - specify --force to overwrite"; exit 1) if (File.exist?(header) && !force)

def write(template, output)
	puts "Writing #{output}"
	File.open output, "w" do |file|
		file.write ERB.new(File.read(template), nil, "%>").result
	end
end

write source_template, source
write header_template, header




