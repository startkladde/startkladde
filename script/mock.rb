#!/usr/bin/env ruby

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

# Source may include:
#   * Entities: <source>&amp;Encoding:</source>
#   * HTML: <source>&lt;html&gt;Both entries must refer to the same person. All flights of the wrong person will be assigned to the correct person. &lt;font color=&quot;#FF0000&quot;&gt;Warning: this action cannot be undone.&lt;/font&gt; Continue?&lt;/html&gt;</source>


filter=ARGV[0]
filename=ARGV[1]

if filter.nil? or filter.empty? or filename.nil? or filename.empty?
	puts "Usage: #{$0} filter_program filename.ts"
	puts "Example: #{$0} 'tr [:lower:] [:upper:]' translations/startkladde_sc.ts"
	exit 1
end

puts "Mocking #{filename}"

# TODO handle XML entities and HTML
def mock(string)
	string.upcase
end

source=''
inSource=false
inTranslation=false
numerusMessage=false

# line includes the trailing newline, this is important for assembling
# multi-line messages
File.open(filename).each { |line|
	translationStart=false
	translationEnd=false

	if line =~ /^\s*<message/
		# New message - reset message state
		source=''
		inSource=false
		inTranslation=false
		# Determine whether it is a numerus message
		numerusMessage=!((line =~ /numerus="yes"/).nil?)

	elsif line =~ /^\s*<source>(.*)<\/source>\s*$/
		# Single-line source string
		source=$1
	elsif line =~ /^\s*<source>(.*)$/m
		# Start of multi-line source string
		source=$1
		inSource=true
	elsif line =~ /(.*)<\/source>\s*$/m
		# End of multi-line translation
		source+=$1
		inSource=false
	elsif inSource
		# Middle of multi-line source string
		source+=line

	elsif line =~ /^\s*<translation.*type="unfinished".*>.*<\/translation>\s*$/
		# Single-line translation (we're only interested in 'unfinished' ones)
		translationStart=true
		translationEnd=true
	elsif line =~ /^\s*<translation.*type="unfinished"/
		# Start of multi-line translation
		translationStart=true
		inTranslation=true
	elsif line =~ /<\/translation>\s*$/ and inTranslation
		# End of multi-line translation - only if we're in a translation,
		# because a translation without type=unfinished does not count as a
		# translation
		translationEnd=true
		inTranslation=false
	end

	# We may not be in a source and in a translation simultaneously
	raise "inSource and inTranslation" if inSource && inTranslation

	# Put this check at the beginning instead of an else clause so we can
	# output ---line for lines not copied - this is great for development
	if !inTranslation && !translationStart && !translationEnd
		# Copy the line
		puts line
	else
		# Don't copy the line
		puts "---#{line}"

		if translationStart
			# This handles both single and multi-line messages
			# TODO handle numerusMessage
			if numerusMessage
				singularGuess=source.gsub('(s)', '')
				pluralGuess  =source.gsub('(s)', 's')
				puts "+++        <translation type=\"unfinished\">"
				puts "+++            <numerusform>#{mock(singularGuess)}</numerusform>"
				puts "+++            <numerusform>#{mock(pluralGuess)}</numerusform>"
				puts "+++        </translation>"
			else
				puts "+++        <translation type=\"unfinished\">#{mock(source)}</translation>"
			end
		end
	end
}

