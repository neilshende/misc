import sys
import re
doc = []
i = 0
j = 0
maxj = 0
for line in sys.stdin:
     doc.append([])
     wordList = re.sub("[^\w]", " ",  line).split()
     for word in wordList:
         doc[i].append(word) 
         j += 1
     i += 1
     if maxj == 0:
        maxj = j
     if j != maxj:
        sys.stderr.write("Error: rows should be of same dimension\n")
        sys.exit(1)
     j = 0
     
ii = 0
jj = 0
while jj < maxj:
   while ii < i: 
        print (doc[ii][jj]),
        ii += 1
   print 
   jj += 1
   ii = 0
