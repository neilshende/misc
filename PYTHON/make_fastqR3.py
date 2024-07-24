#!/bin/python3
import sys
import gzip


Read1 = sys.argv[1]
Read3 = sys.argv[2]
out4 = b"IIIIIIIIII\n"
file1 = gzip.open(Read1)
f3 = gzip.open(Read3, 'wb')
line1 = file1.readline()

while line1:
    #out2 = bytes(((str(line1).split(' ')[1]).split(',')[0]) + "\n", encoding = 'utf-8')
    sl = str(line1)
    ss1 = sl.split(' ')[1]
    ss2 = ss1.split(',')[0]
    ss2n = ss2 + "\n"
    out2 = bytes(ss2n, encoding = 'utf-8')

    line2 = file1.readline()
    line3 = file1.readline()
    line4 = file1.readline()
    f3.write(line1)
    f3.write(out2)
    f3.write(line3)
    f3.write(out4)
    line1 = file1.readline()

file1.close()
f3.close()
