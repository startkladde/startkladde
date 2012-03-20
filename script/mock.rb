#!/usr/bin/env ruby

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

filter=ARGV[0]
filename=ARGV[1]

if filter.nil? or filter.empty? or filename.nil? or filename.empty?
	puts "Usage: #{$0} filter_program filename.ts"
	puts "Example: #{$0} 'tr [:lower:] [:upper:]' translations/startkladde_sc.ts"
	exit 1
end

data=File.read(filename)

#regexp=/(<source>(.*)<\/source>.*<translation)[^>]*><\/translation>/m
regexp=/(<source>(.*?)<\/source>.*?<translation[^>]*?>)(<\/translation>)/m

def mock(string)
	string.upcase
end

data.gsub!(regexp) { |match|
	m1=$1
	m2=$2
	m3=$3

	if match. =~ /&/i # FIXME properly handle & - don't touch XML entities (&xxx;), but do translate strings with &
		match
	else
		"#{m1}#{mock(m2)}#{m3}"
	end
}

puts data

