import psutil

p = psutil.Popen(["python", "-c", "print 'hi'"])

# Get all processes
processes = psutil.process_iter()

# Kill all processes with the name "python"
for process in processes:
    if process.name() == "python":
        process.kill()

p.wait()
