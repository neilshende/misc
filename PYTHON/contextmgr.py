class foobar:
    def __init__(self, val):
        self.fooval = val

    def __del__(self):
        print("foobar destroyer")

    def foo(self):
        print(f"my value is {self.fooval}")

class MyContextManager:
    def __init__(self, val):
        print("initializing")
        self.foobar = foobar(val)

    def __del__(self):
        print("destroying context manager")

    def __enter__(self):
        print("Entering the context")
        # Perform setup actions
        return self.foobar

    def __exit__(self, exc_type, exc_val, exc_tb):
        print("Exiting the context")
        # Perform cleanup actions
        del self.foobar

with MyContextManager(911) as mgr:
    # Code within the context
    print("inside with")
    mgr.foo()

print("outside with")
