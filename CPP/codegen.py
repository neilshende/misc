#cat a.cpp | python ~/git/ci/codegen.py '<< "' ' \n"'
import sys
import re
doc = []
for line in sys.stdin:
   doc.append(line.rstrip('\n'))
print "std::cout "
for i in range(len(doc)):
   print sys.argv[1]+doc[i].replace('\\', '\\\\').replace('"', '\\"')+sys.argv[2]
print ";"
