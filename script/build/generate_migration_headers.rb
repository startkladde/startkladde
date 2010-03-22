#!/usr/bin/env ruby

require 'erb'

list   = "build/migrations.h"
header = "build/migrations_headers.h"

list_template   = "src/db/migration/migrations.h.erb"
header_template = "src/db/migration/migrations_headers.h.erb"

print "Generating migrations lists..."

(puts "error: template #{  list_template} does not exist"; exit 1) if !File.file?(  list_template)
(puts "error: template #{header_template} does not exist"; exit 1) if !File.file?(header_template)


# ["1_foo", "2_bar"]
full_names=Dir["src/db/migrations/Migration_*.h"].map { |filename|
	filename.sub /^.*\/Migration_(.*).h$/, '\\1'
}

# [ { :version=>1, :name=>"foo" }, { :version=>2, :name=>"bar" } ]
migrations=full_names.inject([]) do |migrations, full_name|
	version, name = full_name.scan(/^([0-9]+)_([_a-zA-Z0-9]+)$/).first

	if !version
		puts "error: illegal migration version: #{full_name}"
		exit 1
	end

	version=version.to_i

	if dupe=migrations.find { |migration| migration[:version]==version }
		puts "error: duplicate migration version: #{full_name} and #{dupe[:full_name]}"
		exit 1
	end
	
	if dupe=migrations.find { |migration| migration[:name]==name }
		puts "error: duplicate migration name: #{full_name} and #{dupe[:full_name]}"
		exit 1
	end
	
	migrations << { :version=>version, :name=>name, :full_name=>full_name }
end

migrations = migrations.sort_by { |migration| migration[:version] }

if migrations.empty?
	puts "no migrations found"
	exit 0
end

puts "OK, #{migrations.size} migrations found"

def write(template, output)
	puts "Writing #{output}"
	File.open output, "w" do |file|
		file.write ERB.new(File.read(template), nil, "%>").result
	end
end

write list_template, list
write header_template, header

