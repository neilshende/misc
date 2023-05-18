from ctypes import *
import struct
import sys
so_file = "./my_functions.so"
my_functions = CDLL(so_file)
print(my_functions.square(10))
#hopefully we don't need to mess with:
sys.byteorder
struct.pack("hil",1,2,3) #half, int, long
