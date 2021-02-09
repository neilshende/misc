import urllib
import re
import sys
import os

file = sys.argv[1]
buffer = "This is a test"
fd = open(file, "w+")
fd.write(buffer)
fd.close()
