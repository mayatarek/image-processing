# the client sends images to the server manually through a while loop
# the streaming framewrok will do that automatically
#it will take a micro-batch of images and will call gRPC client to send them to server 
# what is edited here is: 1) removed the loop that sends images 2) made the request logic a function that can be called for each image in the micro-batch
#3) removed the counting req part 4)  the old client saves all resposnes in ram and when the loop ends they get saved to disk
# since spark sends a batch of images, we can run out of ram, so each image will be written in instant after request
#4) adjusted the csv writer cuz spark runs process batch in parallel and that can cause the csv to be inccorect
# each write now uses append 

import grpc
import time
import itertools
import os
import csv 
from datetime import datetime

import edge_pb2
import edge_pb2_grpc

from pyspark.sql import SparkSession
from pyspark.sql.functions import col
from pyspark.sql.types import StructType, StructField, BinaryType, StringType, TimestampType, LongType



# han create el servers dol ta7t
SERVERS = ["localhost:50051", "localhost:50052"]
channels = [grpc.insecure_channel(s) for s in SERVERS]

stubs = [edge_pb2_grpc.EdgeServiceStub(c) for c in channels]

# this will help to round-robin the servers, mohema lel failover mechanism wel load balancing
stub_cycle = itertools.cycle(stubs)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
#spark handles that
# IMAGE_PATH = os.path.join(BASE_DIR, "..", "data", "cat.jpeg")

OUTPUT_DIR = os.path.join(BASE_DIR, "..", "output")
os.makedirs(OUTPUT_DIR, exist_ok=True)


# with open(IMAGE_PATH, "rb") as f:
#     image = f.read() #spark replced this with image_bytes = row.content

NORMAL_TIMEOUT = 10

# start = time.time()
# request_count = 0
# responses = [] 
MAX_RETRIES = 5
RATE_LIMIT_DELAY = 0.5 # adjustable input rate limit delay in seconds (sleeps 0.5 between requests)


# the adjusted csv   
CSV_PATH = "metrics.csv"

if not os.path.exists(CSV_PATH):
    with open(CSV_PATH, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Timestamp", "Latency", "Status"])



def send_image_request(image, current_req, timeout_val=NORMAL_TIMEOUT):    
        
        retry_time = 0 
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
                ts_str = datetime.now().strftime("%H:%M:%S.%f")[:-3] # timestamp for logging
                
                with open(CSV_PATH, "a", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerow([time.time(), total_time, "Success"])

             
                # writer.writerow([time.time(), total_time, "Success"]) # record successful response in CSV
                
                # responses.append((current_req, resp.edges))
                # write each image to disk 
                output_path = os.path.join(OUTPUT_DIR, f"response{current_req}.jpeg")
                with open(output_path, "wb") as f:
                    f.write(resp.edges)                

                print(f"[{ts_str}] OK latency={resp.latency:.3f}s total={total_time:.3f}s (via {SERVERS[stub_idx]})") # log with timestamp and server info
                success = True
                break

            except Exception as e:
                retry_time = time.time() - t0
                print(f"Exception occurred: {e}")  # prints the actual error message
                # optional: print full traceback
                import traceback
                traceback.print_exc()
                
                import traceback
                traceback.print_exc()
                # writer.writerow([time.time(), retry_time, "Failure"]) # record failed attempt in CSV
                print(f"Server {SERVERS[stub_idx]} failed on attempt {attempt + 1} → retrying in {retry_time:.3f}s")
                time.sleep(0.2)


                

        if not success:
            print(f"All {MAX_RETRIES} retries failed for req {current_req} → skipping")
            with open(CSV_PATH, "a", newline="") as f:
                writer = csv.writer(f)
                writer.writerow([time.time(), retry_time, "Failure"])

        #will be controlled by streaming framework
        # request_count += 1  
        #spark handles that with max files trigger
        # time.sleep(rate_delay) # rate limiting delay
        
        return success


# for req_num, edges_bytes in responses:
#     filename = f"response{req_num}.jpeg"
#     output_path = os.path.join(OUTPUT_DIR, filename)
#     with open(output_path, "wb") as f:
#         f.write(edges_bytes)

#starting spark, the spark session biulder is what strats our session so we can start reading data and stream
# gave the session a name, if there a sark session runing use it, if not create one
spark = SparkSession.builder.appName('Edge_Detection_Streaming').getOrCreate()

# next step is to read the stream of files
#made a schema since spark needs it to match the format of the binary file
file_schema = StructType(
    [
    StructField("path", StringType(), True),
    StructField("modificationTime", TimestampType(), True),
    StructField("length", LongType(), True),
    StructField("content", BinaryType(), True)
    ]
)
 
# made the micro batch 10 images
input = spark.readStream.format('binaryFile').option("path", "../data").option("maxFilesPerTrigger", 10).schema(file_schema).load() 

# next we process the micro batch we have 
# each micro batch is a df (spark give it to us with a batch id), every row corresponds to an image
# each image have its row

# def process_batch(batch_df, batch_id):
#     #converting the df in spark memory into python rows using collect()
#     #each row represents 1 image as we said above 
#     for row in batch_df.collect():
#         image_bytes = row.content 
#         print("sendng image request")
    
#         send_image_request(image_bytes, current_req)
#         print("request ended")
request_count = 0      
        
def process_batch(batch_df, batch_id):
    global request_count
    for row in batch_df.collect():
        print(f"sending request #{request_count}")
        image_bytes = row.content
        current_req = request_count  # small integer ID
        request_count += 1
        send_image_request(image_bytes, current_req)
    print("process batch done")       
        
# making spark call the process_batch function we made
# reads images (input mentioned above) and calls the processing function on each micro batch
#query is the streaning job starting 
query = input.writeStream.foreachBatch(process_batch).start()

query.awaitTermination() #keep the sream runing