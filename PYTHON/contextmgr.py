class foobar:
    def __init__(self, val):
        self.fooval = val

    def __del__(self):
        print("foobar destroyer")

    def foo(self):
        print(f"my value is {self.fooval}")

class MyContextManager:
    def __init__(self, val):
        #only needed if you are going to return an object in __enter__
        #build that object here.
        print("initializing")
        self.foobar = foobar(val)

    def __del__(self):  #not needed.
        print("destroying context manager")

    def __enter__(self):
        print("Entering the context")
        # Perform setup actions
        return self.foobar

    def __exit__(self, exc_type, exc_val, exc_tb):
        # Perform cleanup actions
        print("Exiting the context")
        #foobar will be deleted automatically.
        # deleting foobar here or in __del__ has
        # no effect as we have returned it to
        # calling code and it holds a reference.
        #Once calling code exits the with clause,
        # foobar will be deleted.

with MyContextManager(911) as mgr:
    # Code within the context
    print("inside with")
    mgr.foo()

print("outside with")
