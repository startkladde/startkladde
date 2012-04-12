#!/usr/bin/env ruby

require 'pathname'

dir=Pathname.new(File.dirname(__FILE__))

# Don't use system:
#   * on Unixes, we have to use the absolute path
#   * on Windowses, we have to invoke the interpreter explicityly, but it may
#     not be on the path or may have a different name
# We still have to use the absolute path, though, because it doesen't work on
# Windows otherwise.
require dir.join('generate_version_h.rb').expand_path.to_s
require dir.join('generate_current_schema_cpp.rb').expand_path.to_s
