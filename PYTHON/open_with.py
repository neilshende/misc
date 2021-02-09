import sys
from contextlib import contextmanager
@contextmanager
def opened_w_error(filename, mode="r"):
    try:
        f = open(filename, mode)
    except IOError as err:
        yield None, err
    else:
        try:
            yield f, None
        finally:
            f.close()

file=sys.argv[1]
with opened_w_error(file, "a") as (f, err):
    if err:
        print("IOError: ", err)
    else:
        f.write("This is a test\n")
