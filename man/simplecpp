#!/usr/bin/env python
#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
# Contributors to the Freedoom project.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of the freedoom project nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# simple cpp-style preprocessor
#
# Understands most features of the C preprocessor, including:
#    #if .. #elif .. #else .. #endif
#        - with expressions
#    #ifdef
#    #define
#    #include
#

import collections
import io
import sys
import re

debug = False
defines = collections.defaultdict(lambda: False)

command_re = re.compile(r"\#(\w+)(\s+(.*))?")
include_re = re.compile(r"\s*\"(.*)\"\s*")
define_re = re.compile(r"\s*(\S+)\s*(.*?)\s*$")

def debug_msg(message):
	if debug:
		sys.stderr.write(message)

# Parse command line options

def parse_cmdline():
	for arg in sys.argv[1:]:
		if not arg.startswith("-D"):
			continue

		name = arg[2:]
		if '=' in name:
			name, value = name.split('=', 1)
		else:
			value = True

		defines[name] = value

def parse_stream(stream):
	result = read_block(stream, False)

	if result is not None:
		raise Exception("Mismatched #if in '%s'" % stream.name)

def parse_file(filename):
	f = io.open(filename, encoding='UTF-8')

	try:
		parse_stream(f)
	finally:
		f.close()

# Evaluate an expression using Python's eval() function.

def eval_expr(expr):
	expr = expr.replace("||", " or ")   \
	           .replace("&&", " and ")  \
	           .replace("!", "not ")

	code = compile(expr, "", "eval")
	result = eval(code, {}, defines)
	return result

# #include

def cmd_include(arg):
	# Extract the filename

	match = include_re.match(arg)

	if not match:
		raise Exception("Invalid 'include' command")

	filename = match.group(1)

	# Open the file and process it

	parse_file(filename)

# #define

def cmd_define(arg):
	match = define_re.match(arg)
	name = match.group(1)
	value = match.group(2)
	if value == '':
		value = True

	defines[name] = value

# #undef

def cmd_undef(arg):
	if arg in defines:
		del defines[arg]

# #ifdef/#ifndef

def cmd_ifdef(arg, command, stream, ignore):

	# Get the define name

	debug_msg("%s %s >\n" % (command, arg))

	# Should we ignore the contents of this block?

	sub_ignore = not eval_expr(arg)

	if "n" in command:
		sub_ignore = not sub_ignore

	# Parse the block

	result, newarg = read_block(stream, ignore or sub_ignore)

	debug_msg("%s %s < (%s)\n" % (command, arg, result))

	# There may be a second "else" block to parse:

	if result == "else":
		debug_msg("%s %s else >\n" % (command, arg))
		result, arg = read_block(stream, ignore or (not sub_ignore))
		debug_msg("%s %s else < (%s)\n" % (command, arg, result))

	if result == "elif":
		debug_msg("%s %s elif %s>\n" % (command, arg, newarg))
		cmd_ifdef(newarg, "if", stream, ignore or (not sub_ignore))
		result = "endif"

	# Should end in an endif:

	if result != "endif":
		raise Exception("'if' block did not end in an 'endif'")

commands = {
	"include" : cmd_include,
	"define"  : cmd_define,
	"undef"   : cmd_undef,
	"if"      : cmd_ifdef,
	"ifdef"   : cmd_ifdef,
	"ifn"     : cmd_ifdef,
	"ifndef"  : cmd_ifdef,
}

# Recursive block reading function
# if 'ignore' argument is 1, contents are ignored

def read_block(stream, ignore):

	for line in stream:

		# Remove newline

		line = line[0:-1]

		# Check if this line has a command

		match = command_re.match(line)

		if match:
			command = match.group(1)
			arg = match.group(3)

			if command in ("else", "elif", "endif"):
				return (command, arg)
			elif command not in commands:
				raise Exception("Unknown command: '%s'" % \
						command)

			# Get the callback function.

			func = commands[command]

			# Invoke the callback function. #ifdef commands
			# are a special case and need extra arguments.
			# Other commands are only executed if we are not
			# ignoring this block.

			if func == cmd_ifdef:
				cmd_ifdef(arg, command=command,
					       stream=stream,
					       ignore=ignore)
			elif not ignore:
				func(arg)
		else:
			if not ignore:
				for key, value in defines.items():
					if isinstance(value, str):
						line = line.replace(key, value)
				print(line)

parse_cmdline()
parse_stream(sys.stdin)

