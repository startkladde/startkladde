#!/usr/bin/env ruby

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

# Relevant cases:
# Basic case:
#    <message>
#        <location filename="../src/gui/widgets/AcpiWidget_libacpi.cpp" line="68"/>
#        <location filename="../src/gui/widgets/AcpiWidget_libacpi.cpp" line="70"/>
#        <source>Battery: </source>
#        <comment>With traling space</comment>
#        <translation type="unfinished"></translation>
#    </message>
# Multiline text:
#	<message>
#        <source>Logging in as root failed. The given password
#may not be correct.
#Please enter the password for the database user root:</source>
#        <translation type="unfinished"></translation>
#    </message>
# Numerus message:
#	<message numerus="yes">
#        <location filename="../src/gui/windows/objectList/FlightListWindow.cpp" line="128"/>
#        <source>%1: %n flight(s)</source>
#        <translation type="unfinished">
#            <numerusform></numerusform>
#        </translation>
#    </message>
#    <message numerus="yes">
#        <location filename="../src/gui/windows/objectList/FlightListWindow.cpp" line="214"/>
#        <source>%n flight(s) exported</source>
#        <translation>
#            <numerusform>%n Flug exportiert</numerusform>
#            <numerusform>%n Flüge exportiert</numerusform>
#        </translation>
#    </message>

# Souce may include:
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

#data=File.read(filename)

#regexp=/(<source>(.*)<\/source>.*<translation)[^>]*><\/translation>/m
#regexp=/(<source>(.*?)<\/source>.*?<translation[^>]*?>)(<\/translation>)/m

def mock(string)
	string.upcase
end


message=''
messageComplete=false

File.open(filename).each { |line|
	message=''
	messageComplete=false

	if line =~ /^\s*<source>(.*)<\/source>\s*$/
		# Single-line source string
		message=$1
		messageComplete=true
	elsif line =~ /^\s*<source>(.*)$/
		message=$1
		messageComplete=false
	end

	puts line

	if messageComplete
		puts "--- got message: #{message.inspect}"
	end

}

#data.gsub!(regexp) { |match|
#	m1=$1
#	m2=$2
#	m3=$3
#
#	if match. =~ /&/i # FIXME properly handle & - don't touch XML entities (&xxx;), but do translate strings with &
#		match
#	else
#		"#{m1}#{mock(m2)}#{m3}"
#	end
#}

puts data

