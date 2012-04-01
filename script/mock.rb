#!/usr/bin/env ruby

# Next:
#   * output to file instead of stdout
#   * configurable +++/---
#   * entity/HTML/%n handling
#   * call mock filter

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

# Source may include:
#   * Entities: <source>&amp;Encoding:</source>
#   * HTML: <source>&lt;html&gt;Both entries must refer to the same person. All flights of the wrong person will be assigned to the correct person. &lt;font color=&quot;#FF0000&quot;&gt;Warning: this action cannot be undone.&lt;/font&gt; Continue?&lt;/html&gt;</source>


##########
## Util ##
##########

class NilClass; def blank?; true  ; end; end
class String  ; def blank?; empty?; end; end

class String
	def map_lines
		self.split(/\n\r?/).map { |line| yield(line) }.join("\n")
	end
end


###############
## Outputter ##
###############

class DiffOutputter
	def output(lines); puts lines.map_lines { |line| "+++"+line }; end
	def copy  (lines); puts lines                                ; end
	def eat   (lines); puts lines.map_lines { |line| "---"+line }; end
	def close; end
end

class PlainOutputter
	def output(lines); puts lines; end
	def copy  (lines); puts lines; end
	def eat   (lines); end
	def close; end
end


###################
## String mocker ##
###################

class UpcaseStringMocker
	def mock(string)
		string.upcase
	end
end

# A wrapper for another String mocker, for filtering out XML entities, HTML
# tags, %n and & mnemonics
# Currently simply does not mock strings containing &
class QtStringMocker
	def initialize(mocker)
		@mocker=mocker
	end

	def mock(string)
		# TODO XML
		# TODO HTML
		# TODO %n
		# TODO &
		if string=~/&/
			string
		else
			@mocker.mock(string)
		end
	end
end


#################
## File mocker ##
#################

# Parses and mocks a .ts file
class TsFileMocker
	def initialize(stringMocker, outputter)
		@stringMocker=stringMocker
		@outputter=outputter

		@sourceString=''
		@inSource=false
		@inTranslation=false
		@numerusMessage=false
	end

	def processLine(line)
		translationStart=false
		translationEnd=false

		if line =~ /^\s*<message/
			# New message - reset message state
			@sourceString=''
			@inSource=false
			@inTranslation=false
			# Determine whether it is a numerus message
			@numerusMessage=!((line =~ /numerus="yes"/).nil?)

		elsif line =~ /^\s*<source>(.*)<\/source>\s*$/
			# Single-line source string
			@sourceString=$1
		elsif line =~ /^\s*<source>(.*)$/m
			# Start of multi-line source string
			@sourceString=$1
			@inSource=true
		elsif line =~ /(.*)<\/source>\s*$/m
			# End of multi-line translation
			@sourceString+=$1
			@inSource=false
		elsif @inSource
			# Middle of multi-line source string
			@sourceString+=line

		elsif line =~ /^\s*<translation.*type="unfinished".*>.*<\/translation>\s*$/
			# Single-line translation (we're only interested in 'unfinished' ones)
			translationStart=true
			translationEnd=true
		elsif line =~ /^\s*<translation.*type="unfinished"/
			# Start of multi-line translation
			translationStart=true
			@inTranslation=true
		elsif line =~ /<\/translation>\s*$/ and @inTranslation
			# End of multi-line translation - only if we're in a translation,
			# because a translation without type=unfinished does not count as a
			# translation
			translationEnd=true
			@inTranslation=false
		end

		# We may not be in a source and in a translation simultaneously
		raise "inSource and inTranslation" if @inSource && @inTranslation

		# Output new text and determine whether to copy the line. The line is
		# not copied immediately because we want to be able to output ---line
		# if it is not copied, this is great for development.
		copyLine=false
		if translationStart
			# This covers both single and multi-line messages
			# TODO check multi-line numerus message
			if @numerusMessage
				singularGuess=@sourceString.gsub('(s)', '')
				pluralGuess  =@sourceString.gsub('(s)', 's')
				@outputter.output "        <translation type=\"unfinished\">"
				@outputter.output "            <numerusform>#{@stringMocker.mock(singularGuess)}</numerusform>"
				@outputter.output "            <numerusform>#{@stringMocker.mock(pluralGuess)}</numerusform>"
				@outputter.output "        </translation>"
			else
				@outputter.output "        <translation type=\"unfinished\">#{@stringMocker.mock(@sourceString)}</translation>"
			end
		elsif !@inTranslation && !translationEnd
			copyLine=true
		end

		# Copy the line if we determined to do so before
		if copyLine
			@outputter.copy line
		else
			@outputter.eat line
		end
	end

	def processFile(filename)
		# line includes the trailing newline, this is important for assembling
		# multi-line messages
		File.open(filename).each { |line|
			self.processLine(line)
		}
	end
end


##########
## Test ##
##########

def testMocker
	mocker=UpcaseStringMocker.new
	mocker=QtStringMocker.new(mocker)
	puts mocker.mock("foo")
	puts mocker.mock("foo&lt;bar&gt;")
end


##########
## Main ##
##########

def help
	puts "Usage:"
	puts "    #{$0} --test"
	puts "    #{$0} mock_filter filename.ts"
	puts "Example: #{$0} 'tr [:lower:] [:upper:]' translations/startkladde_sc.ts"
	exit 1
end

# Parse parameters
while arg=ARGV.shift
	case arg
	when '--test': test=true
	when /^-/: puts "Unknown parameter #{arg}"; help
	else
		filter=arg
		filename=ARGV.shift
		if !ARGV.empty?
			puts "Extra parameters: #{ARGV}"
			help
		end
	end
end

if test
	testMocker
	exit 0
end

if filter.blank?
	puts "No filter specified"
	help
elsif filename.blank?
	puts "No file name specified"
	help
end

if File.exist?(filename)
	puts "Mocking #{filename}"

	stringMocker=UpcaseStringMocker.new
	stringMocker=QtStringMocker.new(mocker)
	outputter=DiffOutputter.new
	fileMocker=TsFileMocker.new(stringMocker, outputter)

	fileMocker.processFile(filename)
else
	puts "File #{filename} does not exist"
end


