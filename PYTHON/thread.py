import threading
import time

def thread_function(thread_name, delay):
    """Simulates work with a delay."""
    print(f"Thread {thread_name} starting")
    time.sleep(delay)
    print(f"Thread {thread_name} finished")

threads = []
for i in range(3):
    thread = threading.Thread(target=thread_function, args=(f"Thread-{i}", i + 1))
    threads.append(thread)
    thread.start()

# Wait for all threads to finish
for thread in threads:
    thread.join()

print("All threads completed!")
