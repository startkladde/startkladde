#!/usr/bin/env ruby

# Test cases: 
#   line comment with line continuation 
#   /* ... */// foo


def scan_cpp(text)
	pos=0
	offset=0

	# States: :default, :string_literal, :block_comment, :line_comment
	state=:default

#                |   |       |                   |         |   |   index of separator
# v              v    v      v                     v       v    v  start of segment
# >               >   >        >                   >        >   >  start position for next search
# void foo (); a="bar"; b=2; /* more assignments */ return "baz";
# ---------------sssss-------bbbbbbbbbbbbbbbbbbbbbb--------sssss-
#
# void foo (); a="bar"; // assignment|b=2; /* more assignments */ return "baz";


	while true
		case state
		when :default
			next_pos=text.index(/( " | \/\/ | \/\* )/x, pos+offset)
			case $1
			when '"' then next_state=:string_literal; offset=0
			when '//' then next_state=:line_comment; offset=0
			when '/*' then next_state=:block_comment; offset=0
			else raise "Unhandled input #{$1.inspect}"
			end
		when :string_literal
			next_pos=text.index(/( " )/x, pos+offset)
			case $1
			when '"' then next_state=:default; offset=1
			else raise "Unhandled input #{$1.inspect}"
			end
		when :block_comment
			next_pos=text.index(/( \*\/ )/x, pos+offset)
			case $1
			when '*/' then next_state=:default; offset=2
			else raise "Unhandled input #{$1.inspect}"
			end
		when :line_comment
			next_pos=text.index(/( $ )/x, pos+offset)
			case $1
			when '*/' then next_state=:default; offset=2
			else raise "Unhandled input #{$1.inspect}"
			end
		else raise "Unhandled state #{state.inspect}"
		end

		puts "#{pos}-#{next_pos} - #{state.inspect} - #{text[pos...next_pos].inspect}"
		pos=next_pos
		state=next_state
	end

	
#	p i
#	p $1




end

text="void foo (); a=\"bar\"; b=2; /* more assignments */ return \"baz\";"


#text=<<EOF
#void foo ();
#a="bar"; // assignment
#b=2; /* more assignments */ return "baz";
#EOF


puts text
puts
scan_cpp text



