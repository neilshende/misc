from ctypes import *
import struct
import sys
so_file = "./my_functions.so"
my_functions = CDLL(so_file)
j = c_int(99)
variable = c_int(10)

ptr = pointer(variable)

print(my_functions.square(10, pointer(j)))

print("j is ", j)
#hopefully we don't need to mess with:
sys.byteorder
struct.pack("hil",1,2,3) #half, int, long
