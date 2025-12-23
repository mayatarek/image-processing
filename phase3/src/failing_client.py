# import edge_pb2
# import e

import grpc
import time
import itertools
import os

import edge_pb2
import edge_pb2_grpc

#han create el servers dol ta7t
SERVERS = ["localhost:50051", "localhost:50052"]
channels = [grpc.insecure_channel(s) for s in SERVERS]

stubs = [edge_pb2_grpc.EdgeServiceStub(c) for c in channels]

#this will help to round-robin the servers, mohema lel failover mechanism wel load balancing
stub_cycle = itertools.cycle(stubs)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATH = os.path.join(BASE_DIR, "..", "data", "cat.jpeg")

with open(IMAGE_PATH, "rb") as f:
    image = f.read()

NORMAL_TIMEOUT = 10
DISRUPTION_TIMEOUT = 0.1
better_timeout = 0.5

def better_network(meow):
    if meow <= 5:
        return meow*2
    return 10

start = time.time()
request_count = -1
responses = [] 

while time.time() - start < 60:
    stub = next(stub_cycle)
    t0 = time.time()
    try:
        request_count += 1
        
        ##for the simulated failure
        elapsed = time.time() - start
        if 20<= elapsed:
            print("Simulating network getting better")
            
            print(f"timeout is {better_timeout}")
            timeout_val = better_timeout
            better_timeout = better_network(better_timeout)
        elif 10 <= elapsed:
            print("Simulating network disruption meow yarab yeshta8al")
            timeout_val = DISRUPTION_TIMEOUT
       
        else:
            timeout_val = NORMAL_TIMEOUT

        resp = stub.DetectEdges(
            edge_pb2.ImageRequest(image=image, req_num= request_count),
            timeout=timeout_val
        )

        responses.append((request_count, resp.edges))
        print(f"OK latency={resp.latency:.3f}s total={time.time()-t0:.3f}s")

    except Exception:
        print(f"Server failed → retrying in time {time.time()-t0} ")
        time.sleep(0.2)

OUTPUT_DIR = os.path.join(BASE_DIR, "..", "output")
os.makedirs(OUTPUT_DIR, exist_ok=True)

for req_num, edges_bytes in responses:
    filename = f"response{req_num}.jpeg"
    output_path = os.path.join(OUTPUT_DIR, filename)
    with open(output_path, "wb") as f:
        f.write(edges_bytes)
