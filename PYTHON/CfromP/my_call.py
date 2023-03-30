from ctypes import *
so_file = "./my_functions.so"
my_functions = CDLL(so_file)
print(my_functions.square(10))
