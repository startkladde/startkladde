#!/usr/bin/env ruby

require 'pathname'
#require 'fileutils'
require 'erb'

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
	def generate(template)
		ERB.new(template, nil, "%>").result(binding)
	end
end

begin
	generated_files=[
		GeneratedFile.new("src/db/schema/CurrentSchema.cpp.erb", "build/CurrentSchema.cpp")
	]

	# Change to the project base directory
	Dir.chdir Pathname.new(File.dirname(__FILE__)).join("..").join("..")

	# Create the files
	generator=Generator.new
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

