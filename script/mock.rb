#!/usr/bin/env ruby

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

# Source may include:
#   * Entities: <source>&amp;Encoding:</source>
#   * HTML: <source>&lt;html&gt;Both entries must refer to the same person. All flights of the wrong person will be assigned to the correct person. &lt;font color=&quot;#FF0000&quot;&gt;Warning: this action cannot be undone.&lt;/font&gt; Continue?&lt;/html&gt;</source>

class String
	def map_lines
		self.split(/\n\r?/).map { |line| yield(line) }.join("\n")
	end
end

# TODO handle XML entities and HTML
def mock(string)
	string.upcase
end

# Parses and mocks a .ts file
class TsFileMocker
	def initialize
		@sourceString=''
		@inSource=false
		@inTranslation=false
		@numerusMessage=false
	end

	def write(lines)
		puts lines
	end

	def output(lines)
		puts lines.split(/\n\r?/).map { |line| "+++"+line }.join("\n")
	end

	def copy(lines)
		puts lines
	end

	def eat(lines)
		puts lines.split(/\n\r?/).map { |line| "---"+line }.join("\n")
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
				output "        <translation type=\"unfinished\">"
				output "            <numerusform>#{mock(singularGuess)}</numerusform>"
				output "            <numerusform>#{mock(pluralGuess)}</numerusform>"
				output "        </translation>"
			else
				output "        <translation type=\"unfinished\">#{mock(@sourceString)}</translation>"
			end
		elsif !@inTranslation && !translationEnd
			copyLine=true
		end

		# Copy the line if we determined to do so before
		if copyLine
			copy line
		else
			eat line
		end

	end
end

filter=ARGV[0]
filename=ARGV[1]

if filter.nil? or filter.empty? or filename.nil? or filename.empty?
	puts "Usage: #{$0} filter_program filename.ts"
	puts "Example: #{$0} 'tr [:lower:] [:upper:]' translations/startkladde_sc.ts"
	exit 1
end

puts "Mocking #{filename}"

fileMocker=TsFileMocker.new

# line includes the trailing newline, this is important for assembling
# multi-line messages
File.open(filename).each { |line|
	fileMocker.processLine line
}

