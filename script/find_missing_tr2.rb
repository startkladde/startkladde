#!/usr/bin/env ruby

require 'rubygems'
require 'rainbow'




def scan_cpp(text)
	colors = {
		:default        => :cyan,
		:string_literal => :yellow,
		:block_comment  => :green,
		:line_comment   => :blue
	}

	colored=""

#
#                |   |  |            |     |                   |         |   |   index of separator element
#                1   1  2            1     2                   2         1   1
# <-------------><---><><-----------><----><--------------------><------><--->x  segment boundaries
# >               v   v   v           v      v                   v        v   >  start position for next search
# void foo (); a="bar"; // assignment|b=2; /* more assignments */ return "baz";
# ---------------sssss--lllllllllllll------bbbbbbbbbbbbbbbbbbbbbb--------sssss-


	# :default :string_literal :block_comment :line_comment
	state=:default
	segment_start=0

	begin
		while state
			case state
			when :default
				next_segment_start=text.index(/( " | \/\/ | \/\* )/x, segment_start)
				delimiter=$1
				case delimiter
				when '"'  then next_state=:string_literal
				when '//' then next_state=:line_comment
				when '/*' then next_state=:block_comment
				when nil  then next_state=nil
				else raise "Unhandled input #{delimiter.inspect}"
				end
			when :string_literal
				next_segment_start=text.index(/( " )/x, segment_start+1)+1
				delimiter=$1
				case delimiter
				when '"' then next_state=:default
				else raise "Unhandled input #{delimiter.inspect}"
				end
			when :block_comment
				next_segment_start=text.index(/( \*\/ )/x, segment_start+2)+2
				delimiter=$1
				case delimiter
				when '*/' then next_state=:default
				else raise "Unhandled input #{delimiter.inspect}"
				end
			when :line_comment
				next_segment_start=text.index(/( $ )/x, segment_start)
				delimiter=$1
				case delimiter
				when '' then next_state=:default
				else raise "Unhandled input #{delimiter.inspect}"
				end
			else raise "Unhandled state #{state.inspect}"
			end

			if (next_segment_start!=segment_start)
				next_segment_start=0 if !next_state

				segment=text[segment_start..next_segment_start-1]
				puts "#{segment_start}-#{next_segment_start-1} - #{state.inspect} - #{segment.inspect}"
				colored += segment.color(colors[state])
			end

			segment_start=next_segment_start
			state=next_state
		end
	ensure
		puts
		puts colored
		puts
	end

end

text=<<EOF
// Comment at the beginning

void foo (const QString &x="STRING")
{
	// Simple cases
	FUNCTION (); FUNCTION (); // COMMENT
	FUNCTION (); /* COMMENT */ FUNCTION ();
	FUNCTION ("STRING");

	// Adjacent segments
	"STRING""STRING"/* COMMENT *//* COMMENT */"STRING"// COMMENT

	// String with comment delimiters
	FUNCTION ("before // not a comment, a STRING");
	FUNCTION ("before /* not a comment, a STRING */ after");

	// Comments with string delimiters
	FUNCTION (); // before "not a string, a COMMENT"
	FUNCTION (); /* before "not a string, a COMMENT" after */ FUNCTION ();

	// Comments with other comment delimiters
	FUNCTION (); // /* block comment */ still a comment
	FUNCTION (); /* block comment // still a comment */
	FUNCTION (); /* block comment /* still a comment */

	// TODO: line comment with line continuation 
	// TODO: string with line continuation
	// TODO: string with escaped double quote
	// TODO: string with escaped backslash before the end
}
EOF


puts text
puts
scan_cpp text



