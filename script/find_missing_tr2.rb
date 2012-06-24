#!/usr/bin/env ruby

# TODO:
#   * allow multi-argument functions
#     * QT_TRANSLATE_NOOP, QT_TRANSLATE_NOOP3
#     * qnotrCount
#     * multi-argument tr
#   * make sure that variables starting with ntr_ are passed through a translation function
#   * does not handle multi-line strings (strings with a literal newline)
#     correctly, but that's ugly anyway and can be replaced with C string
#     concatenation
#   * does not handle /* and */ in strings correctly (but // is handled)

$: << File.dirname($0)

require 'rubygems'
require 'rainbow'

require 'cpp_scanner'
require 'token'

class Recognizer
	def initialize(name, verbose, patterns)
		@name=name
		@verbose=verbose
		@patterns=patterns

		@state=0
		@accept=false
	end

	def input_token(token)
		print "Recognizer #{@name} input token #{token.type.inspect} #{token.code.inspect}, " if @verbose
		current_pattern=@patterns[@state]

		if token.match?(*current_pattern)
			print "accepted" if @verbose
			@state+=1
			@accept=true
			@state=0 if @state>=@patterns.size
		else
			print "reset" if @verbose
			@state=0
			@accept=false
		end

		puts if @verbose
	end

	def accept?
		@accept
	end
end

# Returns an array of test lines.
# Each line is an array consting of:
#  * a string of code to test
#  * an array, indicating, for each string in the code, whether it is good (the
#    ground truth)

def test_data
	test_lines=[
		# Simple cases
		['foo ();'                        , []      ], # No quoted text
		['a="moo"'                        , [false] ], # Quoted text
		[],
		# With comments
		['foo ();  // A "commented" string'     , []      ], # Quoted text in comment
		['a="moo"; // A comment'                , [false] ], # Quoted text and comment
		['a="moo"; // A "commented" string'     , [false] ], # Quoted text and quoted text in comment
		['a="moo", 2/3; // A "commented" string', [false] ], # Quoted text and quoted text in comment
		[],
		# With include
		['#include "foo.h"'               , [true]  ], # Regular include
		['    #include "foo.h"'           , [true]  ], # Indented include
		['foo (); // #include "foo.h"'    , []      ], # No quote, include in comment
		['a="moo"; // #include "foo.h"'   , [false] ], # Quoted text, #include in comment
		[],
		# With tr
		['foo=tr ("moo");'                , [true ] ],
		['foo=tr("moo");'                 , [true ] ],
		['foo=notr("moo");'               , [true ] ],
		['foo=qnotr("moo");'              , [true ] ],
		['foo=fauxtr ("moo");'            , [false] ],
		['foo=fauxtr("moo");'             , [false] ],
		['foo=t/*.*/r("moo");'            , [false] ], # Not tr
		['foo=faux/*.*/tr("moo");'        , [true ] ], # Not fauxtr
		[],
		# With two-argument form of tr
		['foo=tr ("moo", "disambiguation")'       , [true, true] ],
		['foo=tr ("moo" "bert", "disambiguation")', [true, true, true] ],
		['foo=tr ("moo", "dis" "ambiguation")'    , [true, true, true] ],
		[],
		# With escaped quotes
		['foo=tr ("foo\\"bar")',           [true ]       ], # Single escaped quote
		['foo=tr ("foo\\"bar\\"baz")',     [true ]       ], # Multiple escaped quotes
		['foo=tr ("foo\\\\"bar\\\\"baz")', [true, false] ], # Now they are not escaped and actually end the string
		[],
		# With more complex tr
		['foo=qApp->translate("context","moo")'                        , [true, true]       ], # No whitespace
		[' foo  =  qApp  ->  translate  (  " context "  ,  " moo "  ) ', [true, true]       ], # Lots of whitespace
		['foo=qApp->translate("context","moo","comment")'              , [true, true, true] ], # 3-argument form
		[],
		# extern "C"
		['extern"C"{}'        , [true] ],
		['extern  "C"  {  }  ', [true] ],
		[],
		# Multi-line strings
		['foo="moo'    , [false] ], # Multi-line string
		['bert";'      , [false] ], # Multi-line string, continued
		['foo=tr ("moo', [true ] ], # Multi-line string
		['bert");'     , [true ] ], # Multi-line string, continued
		['foo=tr('     , []      ], # String on next line
		['"bar");'     , [true ] ], # String on next line
		[],
		# Empty strings
		['a=""'    , [false] ], # Empty string
		['a=" "'   , [false] ], # Empty string
		['a="   "' , [false] ], # Empty string
		[],
		# String concatenation
		['foo=tr ("moo" "bert")'        , [true, true]        ], # String concatenation
		['foo=tr ("moo" "bert"), "bar"' , [true, true, false] ], # Missing after string concatenation
		[],
		# With multiple strings
		['foo (tr ("moo"), tr ("bert"));' , [true , true ] ],
		['foo (("moo"), tr ("bert"));'    , [false, true ] ],
		['foo (tr ("moo"), ("bert"));'    , [true , false] ],
		['foo (tr (42), ("bert"));'       , [false]        ], # This tr may not be associated with that string
	].map { |x| if x==[] then ['', []] else x; end }
end


def check_strings(lines_tokens)
	string_good={}
	string_good.default=false

	recognizers=[
		Recognizer.new("include", false, [
			[:default       , /\#$/     ], # FIXME #$
			[:identifier    , "include"],
			[:string_literal, nil      ]]),
		Recognizer.new("extern \"C\"", false, [
			[:identifier    , "extern"],
			[:string_literal, nil     ]]),
		Recognizer.new("tr", false, [
			[:identifier    , "tr"],
			[:default       , "(" ],
			[:string_literal, nil ],
			[:default       , "," ],
			[:string_literal, nil ]]),
		Recognizer.new("qApp->translate", false, [
			[:identifier    , "qApp"],
			[:default       , "->" ],
			[:identifier    , "translate"],
			[:default       , "(" ],
			[:string_literal, nil ],
			[:default       , "," ],
			[:string_literal, nil ],
			[:default       , "," ],
			[:string_literal, nil ]]),
		Recognizer.new("Single-argument functions", false, [
			[:identifier    , /^(notr|qnotr)$/],
			[:default       , "(" ],
			[:string_literal, nil ],
			[:default       , "," ],
			[:string_literal, nil ]])]


	last_token_was_string=false
	last_token_was_good  =false

	lines_tokens.each { |tokens|
		#CppScanner.pretty_print_tokens tokens; puts
		tokens.each { |token|
			if token.type==:whitespace
				# Ignore whitespace
			else
				if token.type==:string_literal && last_token_was_string
					string_good[token]=true if last_token_was_good
				else
					accept=false

					recognizers.each { |recognizer|
						recognizer.input_token token
						accept=true if recognizer.accept?
					}

					string_good[token]=true if accept

					last_token_was_string=(token.type==:string_literal)
					last_token_was_good  =accept
				end
			end
		}
	}

	return string_good
end

# chomp is a parameter because the string may be wrapped in color codes, so it
# can't be chomped afterwards.
def format_single_line(tokens, string_good, chomp)
	tokens.map { |token|
		code=token.code
		type=token.type

		code=code.chomp if chomp

		if type==:string_literal
			good=string_good[token];
			if    good==true  then color=:green
			else                   color=:red
			end
			code.color(color)
		else
			code
		end
	}.join
end

def print_lines(lines_tokens, string_good, display_all)
	num_lines=lines_tokens.size
	padwidth=(num_lines+1).to_s.size

	lines_tokens.each_with_index { |tokens, index|
		display=display_all

		display ||= tokens.find { |token|
			token.type==:string_literal && string_good[token]==false
		}

		if display
			print (index+1).to_s.ljust(padwidth)
			print ": "
			puts format_single_line(tokens, string_good, true)
		end
	}
end

def perform_unit_tests
	data=test_data

	# Construct the raw text
	text=data.map { |data_line| data_line[0] }.join("\n")

	# Scan the text
	scanner=CppScanner.new
	lines_tokens=scanner.scan_lines_token_list(text)

	# Make sure that we have as many token lines as text lines
	# The text is assembled from the test data lines and then split for
	# tokenization, so this may go wrong.
	raise "Test data has #{data.size} lines, lines_tokens has #{lines_tokens.size} lines" if lines_tokens.size!=data.size

	# Check the strings
	string_good=check_strings(lines_tokens)

	# For each line, extract the string validity and compare them with the ground truth
	data.each_with_index { |data_line, index|
		ground_truth=data_line[1]

		tokens=lines_tokens[index]

		check_result=tokens.select { |token|
			token.type==:string_literal
		}.map { |token|
			string_good[token]
		}

		if check_result==ground_truth
			puts "OK      - #{format_single_line(tokens, string_good, true)}"
		else
			puts "Failure - #{format_single_line(tokens, string_good, true)}       - expected #{ground_truth.inspect}, got #{check_result.inspect}"
		end
	}

end

def check_text(text, print_tokens, print_all)
	# Scan the text, produce tokens
	scanner=CppScanner.new
	lines_tokens=scanner.scan_lines_token_list(text)

	# Print the tokens if requested
	CppScanner.pretty_print_lines_tokens lines_tokens if print_tokens

	# Check the strings
	string_good=check_strings(lines_tokens)

	# Print lines (all or bad only)
	print_lines(lines_tokens, string_good, print_all)

end

mode=unit_tests=false
use_test_data=false
print_tokens=false
print_all=false
filename=nil

while arg=ARGV.shift do
	if    arg=="--test-data" then use_test_data=true
	elsif arg=="--print-all" then print_all=true
	elsif arg=="--print-tokens" then print_tokens=true
	elsif arg=="-t" then unit_tests=true
	else  filename=arg
	end
end

error=false

if filename.nil? && !use_test_data && !unit_tests
	puts "Usage:"
	# FIXME better message, allow multiple files, add -a
	puts "#{$0} [--print-tokens] [--print-all] filename"
	puts "#{$0} [--print-tokens] [--print-all] --test-data (use integrated test data)"
	puts "#{$0} -t (run integrated unit tests)"
	error=true
end

exit 1 if error

if unit_tests
	perform_unit_tests
else
	if use_test_data
		text=test_data.map { |l| l[0] }.join("\n")
	else
		text=File.read(filename)
	end

	check_text text, print_tokens, print_all
end

