import threading
import time

lock = threading.Lock()  # Create a lock object

counter = 0

def increment_counter():
    global counter
    for _ in range(100000):
        with lock:  # Acquire the lock before accessing the shared resource
            counter += 1

threads = []
for _ in range(5):
    thread = threading.Thread(target=increment_counter)
    threads.append(thread)
    thread.start()

for thread in threads:
    thread.join()
print(f"The counter is {counter}")
