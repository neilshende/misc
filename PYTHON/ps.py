import psutil

def graceful(process_id):
   process = psutil.Process(process_id)
   process.send_signal(psutil.SIGTERM)

def killgroup(process_id):
   process = psutil.Process(process_id)
   for child in process.children(recursive=True):
       child.kill()
   process.kill()

def pkill(pname):
    processes = psutil.process_iter()
    for process in processes:
       if process.name() == pname:
            process.kill()

p = psutil.Popen(["python", "-c", "print 'hi'"])
p.wait()
