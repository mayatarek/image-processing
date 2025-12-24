#hena ba2a da el grpc server
#el file da beydefine el server beybehave ezay, w later we will make server replicas bas da bas el file def

import grpc
from concurrent import futures
import time
import threading
import os
import signal
import argparse

import edge_pb2
import edge_pb2_grpc
from sobel import sobel_edge_detection   # MUST be a file

class EdgeService(edge_pb2_grpc.EdgeServiceServicer):
    #hena el class beya5od el request w el context mesh hanesta5demo bas grpc beyrequire en it gets passed
    def DetectEdges(self, request, context):
      try:
        print(f"Received request# {request.req_num}")
        start = time.time()
        #hena request.image 3ashan fel edge.proto sameinaha keda
        edge, edge_latency = sobel_edge_detection(request.image)
        total = time.time() - start

        print(f"Handled request | algo={edge_latency:.3f}s total={total:.3f}s")


        return edge_pb2.ImageResponse(
            edges=edge,
            latency=edge_latency
        )
      except:
        print("something went wrong, sad")

def serve(port):
    #keda benkaryet grpc server 3ady w ben allow up to 4 workers gowa kol server
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=4))

    #da el file men fo2 be2ollo yesta5dem EdgeService lel server
    #el kalam el gameel da hay5ally lamma el client yendah DetectEdges elly gowa el class elly fo2 tetnedeh
    edge_pb2_grpc.add_EdgeServiceServicer_to_server(EdgeService(), server)
    server.add_insecure_port(f"[::]:{port}")
    server.start()
    print(f"Server started on port {port}")
    return server

#this is failure number 1, kill the server
def auto_crash(server, delay_sec):
    time.sleep(delay_sec)
    print(f"Simulating server crash after {delay_sec}s")
    server.stop(grace=0)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("port", type=int, help="Port to run the gRPC server on")
    parser.add_argument("--kill", action="store_true", help="Automatically crash the server")
    parser.add_argument("--count", type=int, default=10, help="Seconds before auto crash")
    parser.add_argument("--recovery", type=int, default=5, help="Seconds to wait before recover after crash")
    args = parser.parse_args()
    
    RECOVERY_DELAY = args.recovery


    while True: 
      server = None 
      try:
            #lamma ne run python server.py 50051, el server.py de argv[0] wel 50051 de argv[1]
            server = serve(args.port)
            if args.kill:
                threading.Thread(target=auto_crash, args=(server, args.count), daemon=True).start()

            server.wait_for_termination()  # Block until stopped (crash aw interrupt b2a)
            print(f"Server on port {args.port} stopped")

      except KeyboardInterrupt:
            print(f"Manual interrupt on port {args.port}")
            if server:
                server.stop(0)
            break

      except Exception as e:
            print(f"Unexpected error on port {args.port}: {e}")

      print(f"Waiting {RECOVERY_DELAY}s before restarting server on port {args.port}...")
      time.sleep(RECOVERY_DELAY)
      print(f"Restarting server on port {args.port}...")

