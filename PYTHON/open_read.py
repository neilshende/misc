import urllib
import re
import sys
import os

file = sys.argv[1]
try:
#with open(file, "r") as fd:
   fd = open(file, "r")
   #
   #contents = fd.read()
   lines = fd.readlines()
   for line in lines:
      print(line, sep='', end='')
   fd.close()
except:
   print("No such file ",sys.argv[1])
