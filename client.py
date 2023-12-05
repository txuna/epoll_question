from socket import *
import threading

def connection():
    sd = socket(AF_INET, SOCK_STREAM)
    sd.connect(("127.0.0.1", 9988))


task = []

for i in range(100):
    t = threading.Thread(target=connection)
    task.append(t)
    t.start()
    
    
for t in task:
    t.join()
