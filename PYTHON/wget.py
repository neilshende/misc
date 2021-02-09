import urllib
import re
import sys
import os

url=sys.argv[1]
request = urllib.urlopen(url).read()
print request
