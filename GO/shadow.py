#!/usr/bin/python3
import sys
import re
from collections import defaultdict

def scan_file(filename):
  """
  Scans a Golang source code file for variable shadowing errors.

  Args:
    filename: The path to the source code file.
  """
  with open(filename, "r") as f:
    source_code = f.read()

  # Regular expression to match variable declarations
  variable_declaration_regex = r"((?P<type>[^\s]+)\s+)?(var|const)\s+(?P<name>\w+)\s+=\s+(?P<value>.+)"

  # Variable shadowing map
  shadow_map = defaultdict(list)

  # Parse the source code
  for line_number, line in enumerate(source_code.splitlines(), start=1):
    #print(f"processing {line_number} {line}")
    match = re.match(variable_declaration_regex, line)
    if match:
      name = match.group("name")
      print(f"Found name {name} at {line_number}")
      # Check for existing variables with the same name
      for enclosing_scope in shadow_map:
        if name in enclosing_scope:
          shadow_map[enclosing_scope].append((line_number, name))
          break

      # Add the current variable to the current scope
      shadow_map[line_number] = []

  # Report shadowing errors
  for line_number, errors in shadow_map.items():
    if errors:
      for error_line_number, error_name in errors:
        print(f"Line {error_line_number}: variable '{error_name}' shadows a previously declared variable on line {line_number}.")

# Example usage
scan_file(sys.argv[1])
