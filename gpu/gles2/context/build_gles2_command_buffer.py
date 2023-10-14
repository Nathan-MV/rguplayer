# Copyright 2023 Admenri.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--file", type=str, help="File path")
parser.add_argument("--output", type=str, help="File path")
parser.add_argument("--header", action="store_true", help="Generate header file")
parser.add_argument("--body", action="store_true", help="Generate defined body file")

args = parser.parse_args()

autogen_comments = '''/*
Autogenerated file - DO NOT EDIT
This file was generated by the //gpu/gles2/build_gles2_command_buffer.py.
*/
\n\n'''

if args.header:
  with open(args.file, "r") as file:
    functions = file.readlines()
  functions = [function.strip() for function in functions]
  converted_functions = []
  converted_functions.append(autogen_comments)
  for function in functions:
    if function != "":
      converted_functions.append("_PFNGL" + function.upper() + "PROC " + function + " = nullptr;\n\n")
  with open(args.output, "w") as file:
    file.writelines(converted_functions)

elif args.body:
  with open(args.file, "r") as file:
    functions = file.readlines()
  functions = [function.strip() for function in functions]
  converted_functions = []
  converted_functions.append(autogen_comments)
  for function in functions:
    if function != "":
      converted_functions.append("{} = static_cast<{}>(GetProc(\"{}\"));\n\n".format(function, "_PFNGL" + function.upper() + "PROC", function))
  with open(args.output, "w") as file:
    file.writelines(converted_functions)
