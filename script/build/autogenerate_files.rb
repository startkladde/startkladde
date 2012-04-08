#!/usr/bin/env ruby

require 'pathname'
#require 'fileutils'
require 'erb'

class MigrationDescription
	attr_reader :directory, :version, :name

	def initialize(directory, version, name)
		@directory=directory
		@version=version
		@name=name
	end

	def cpp_file
		directory.join("Migration_#{version}_#{name}.cpp")
	end

	def h_file
		directory.join("Migration_#{version}_#{name}.h")
	end

	def self.list(directory)
		list=[]
		Pathname.glob(directory.join('Migration_*.h')).each { |pathname|
			basename=pathname.basename.to_s

			if basename =~ /^Migration_(\d+)_(.+)\.h$/
				version=$1
				name=$2

				# TODO test
				raise "Duplicate migration name: #{name}"       if list.find { |migration| migration.name   ==name    }
				raise "Duplicate migration version: #{version}" if list.find { |migration| migration.version==version }

				migration=MigrationDescription.new(directory, version, name)
				raise "File does not exist: #{migration.cpp_file}" unless migration.cpp_file.file?
				raise "File does not exist: #{migration.h_file  }" unless migration.h_file  .file?

				list << migration
			else
				puts "Unrecognized file name #{basename}"
			end
		}
		return list.sort_by { |migration| migration.version }
	end
end

class GeneratedFile
	attr_reader :template, :output

	def initialize(template, output)
		template=Pathname.new(template) unless template.is_a?(Pathname)
		output  =Pathname.new(output  ) unless output  .is_a?(Pathname)
		@template=template
		@output=output
	end
end

class Generator
	def initialize(migrations)
		@migrations=migrations
	end

	def generate(template)
		ERB.new(template, nil, "%>").result(binding)
	end
end

begin
	# Settings
	migrations_path=Pathname.new('src/db/migrations')

	generated_files=[
		# These files use the migrations
		GeneratedFile.new("src/db/migration/migrations.h.erb"        , "build/migrations.h"        ),
		GeneratedFile.new("src/db/migration/migrations_headers.h.erb", "build/migrations_headers.h"),
		GeneratedFile.new("src/migrations.pro.erb"                   , "build/migrations.pro"      ),

		# This does not use the migrations at all
		GeneratedFile.new("src/db/schema/CurrentSchema.cpp.erb", "build/CurrentSchema.cpp")
	]

	# Change to the project base directory
	Dir.chdir Pathname.new(File.dirname(__FILE__)).join("..").join("..")

	# List the migrations
	migrations=MigrationDescription.list(migrations_path)

	if migrations.empty?
		puts "No migrations found"
		exit 1
	end

	puts "#{migrations.size} migrations found:"
	migrations.each { |migration|
		puts "    #{migration.version} - #{migration.name}"
	}

	# Create the files
	generator=Generator.new(migrations)
	generated_files.each { |generatedFile|
		if generatedFile.template.file?
		#if File.file?(generatedFile.template)
			puts "Writing #{generatedFile.output}"


			template=File.read(generatedFile.template)
			output=generator.generate(template)
			generatedFile.output.dirname.mkpath
			File.open(generatedFile.output, "w") { |file| file.write output }
		else
			puts "Template #{generatedFile.template} does not exist" 
		end
	}

rescue RuntimeError => ex
	puts "Error: #{ex}"

end

Dir.chdir "version"
system "ruby make_version.rb"

