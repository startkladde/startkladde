#!/usr/bin/env ruby

require 'cgi'
require 'pty'
require 'tempfile'

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

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

class FileOutputter
	def initialize(filename)
		@file=File.open(filename, 'w')
	end

	def output(lines); @file.puts lines; end
	def copy  (lines); @file.puts lines; end
	def eat   (lines); end
	def close; @file.close; end
end

class QuietOutputter
	def output(lines); end
	def copy  (lines); end
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

	def close
	end
end

class ProcessMocker
	def initialize(command)
		@read,@write,@pid=PTY.spawn(command)
	end

	def mock(string)
		string.lines.map { |line|
			@write.puts line
			@read.readline.chomp
			result=@read.readline.chomp
			result
		}.join("\n")
	end

	def close
		@read.close
		@write.close
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
		string=CGI.unescapeHTML(string)

		# TODO mnemonics (e. g. File&name)
		re=/
			([^<%]+) # A text - anything not containing a < or %
		|
			(< [^>]* >) # An HTML tag
		|
			(%.) # % followed by a single character
		/x
		string=string.scan(re).map { |text, html_tag, percent|
			if text
				#puts "    Text: #{text.inspect}"
				@mocker.mock(text)
			elsif html_tag
				#puts "    HTML: #{html_tag.inspect}"
				html_tag
			elsif percent
				#puts "    Percent: #{html_tag.inspect}"
				percent
			else
				"???"
			end
		}.join

		CGI.escapeHTML(string)
	end

	def close
		@mocker.close
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
	[
		["foo"                                  , "FOO"                                  ],
		["foo&amp;bar&amp;"                     , "FOO&amp;BAR&amp;"                     ],
		["foo&lt;font&gt;bar"                   , "FOO&lt;font&gt;BAR"                   ],
		["foo&lt;/font&gt;bar"                  , "FOO&lt;/font&gt;BAR"                  ],
		["foo&lt;font size=&quot;x&quot;&gt;bar", "FOO&lt;font size=&quot;x&quot;&gt;BAR"],
		["%n foo(s)"                            , "%n FOO(S)"                            ],
		["", ""]
	].each { |source, expected|
		actual=mocker.mock(source)
		if actual==expected
			puts "OK - #{source} - #{actual}"
		else
			puts "fail - #{source} - #{actual}, expected #{expected}"
		end
	}
end


##########
## Main ##
##########

def help
	puts "Usage:"
	puts "    #{$0} [-f|--filter mock_filter] filename.ts"
	puts "        Prints the file with added and removed lines"
	puts "    #{$0} [-f|--filter mock_filter] [-o|--output filename] filename.ts"
	puts "        Writes the result to filename or standard output if filename is -, or no output if filename is empty"
	puts "    #{$0} [-f|--filter mock_filter] --diff filename.ts"
	puts "        Shows a diff"
	puts "    #{$0} [-f|--filter mock_filter] --inplace filename.ts"
	puts "        Overwrites the input file in place"
	puts "    #{$0} --test"
	puts "        Runs integrated unit tests"
	puts
	puts "Example: #{$0} -f 'tr [:lower:] [:upper:]' translations/startkladde_sc.ts"
	puts
	puts "The filter must output exactly one line for each line input."
	exit 1
end

def createStringMocker(filter)
	if filter
		stringMocker=ProcessMocker.new(filter)
	else
		stringMocker=UpcaseStringMocker.new
	end

	QtStringMocker.new(stringMocker)
end

def createOutputter(output)
	if output.nil?
		DiffOutputter.new
	elsif output.empty?
		QuietOutputter.new
	elsif output=="-"
		PlainOutputter.new
	else
		FileOutputter.new(output)
	end
end

# Parse parameters
test=false
diff=false
inplace=false
filenames=[]
while arg=ARGV.shift
	case arg
	when '--test': test=true
	when '--diff': diff=true
	when '--inplace': inplace=true
	when '-f' , '--filter': filter=ARGV.shift; if filter.nil?; puts "Filter missing"; help; end
	when '-o' , '--output': output=ARGV.shift; if output.nil?; puts "Output missing"; help; end
	when /^-/: puts "Unknown parameter #{arg}"; help
	else filenames << arg
	end
end

if test
	testMocker
	exit 0
end

if filenames.empty?
	puts "No files specified"
	help
elsif filenames.size>1
	puts "Too many file specified"
	help
end

filename=filenames[0]
if !File.exist?(filename)
	puts "File #{filename} does not exist"
	exit 1
end

stringMocker=createStringMocker(filter)

if diff
	tempfile=Tempfile.new('mock')
	outputter=FileOutputter.new(tempfile.path)
	TsFileMocker.new(stringMocker, outputter).processFile(filename)
	outputter.close
	system "diff", "-u", filename, tempfile.path
	tempfile.unlink
elsif inplace
	tempfile=Tempfile.new('mock')
	outputter=FileOutputter.new(tempfile.path)
	TsFileMocker.new(stringMocker, outputter).processFile(filename)
	outputter.close
	# Not using FileUtils.mv because that needs to change the permissions and
	# this may not be possible on a shared filesystem.
	File.open(filename, "w") { |file| file.print File.read(tempfile.path) }
	tempfile.unlink
else
	outputter=createOutputter(output)
	TsFileMocker.new(stringMocker, outputter).processFile(filename)
	outputter.close
end



