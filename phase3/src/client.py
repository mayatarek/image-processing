import grpc
import time
import itertools
import os
import csv 

import edge_pb2
import edge_pb2_grpc

# han create el servers dol ta7t
SERVERS = ["localhost:50051", "localhost:50052"]
channels = [grpc.insecure_channel(s) for s in SERVERS]

stubs = [edge_pb2_grpc.EdgeServiceStub(c) for c in channels]

# this will help to round-robin the servers, mohema lel failover mechanism wel load balancing
stub_cycle = itertools.cycle(stubs)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATH = os.path.join(BASE_DIR, "..", "data", "cat.jpeg")

with open(IMAGE_PATH, "rb") as f:
    image = f.read()

NORMAL_TIMEOUT = 10

start = time.time()
request_count = 0
responses = [] 
MAX_RETRIES = 5

with open('metrics.csv', mode='w', newline='') as csv_file: # open CSV file for writing
    writer = csv.writer(csv_file)
    writer.writerow(['Timestamp', 'Latency', 'Status'])

    while time.time() - start < 60:
        timeout_val = NORMAL_TIMEOUT  

        current_req = request_count
        success = False
        for attempt in range(MAX_RETRIES):
            stub = next(stub_cycle)  
            stub_idx = stubs.index(stub) 
            channel = channels[stub_idx]
            
            try:
                grpc.channel_ready_future(channel).result(timeout=0.1)
            except grpc.FutureTimeoutError:
                print(f"Channel {SERVERS[stub_idx]} not READY → skipping to next (attempt {attempt + 1}/{MAX_RETRIES})")
                continue  
            
            t0 = time.time()
            try:
                resp = stub.DetectEdges(
                    edge_pb2.ImageRequest(image=image, req_num=current_req),
                    timeout=timeout_val
                )
                total_time = time.time() - t0
                
             
                writer.writerow([time.time(), total_time, "Success"]) # record successful response in CSV
                
                responses.append((current_req, resp.edges))
                print(f"OK latency={resp.latency:.3f}s total={total_time:.3f}s (via {SERVERS[stub_idx]})")
                success = True
                break

            except Exception:
                retry_time = time.time() - t0
                writer.writerow([time.time(), retry_time, "Failure"]) # record failed attempt in CSV
                print(f"Server {SERVERS[stub_idx]} failed on attempt {attempt + 1} → retrying in {retry_time:.3f}s")
                time.sleep(0.2)

        if not success:
            print(f"All {MAX_RETRIES} retries failed for req {current_req} → skipping")

        request_count += 1  

OUTPUT_DIR = os.path.join(BASE_DIR, "..", "output")
os.makedirs(OUTPUT_DIR, exist_ok=True)

for req_num, edges_bytes in responses:
    filename = f"response{req_num}.jpeg"
    output_path = os.path.join(OUTPUT_DIR, filename)
    with open(output_path, "wb") as f:
        f.write(edges_bytes)