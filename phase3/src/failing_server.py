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
    # server.wait_for_termination()
    while True:
      print(f"Server on port {port} will sleep for 30")
      time.sleep(30)
      print(f"Server on port {port} woke up")

#this is failure number 1, kill the server
def auto_crash(delay_sec):
    time.sleep(delay_sec)
    print(f"Simulating server crash after {delay_sec}s")
    os.kill(os.getpid(), signal.SIGTERM)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("port", type=int, help="Port to run the gRPC server on")
    parser.add_argument("--kill", action="store_true", help="Automatically crash the server")
    parser.add_argument("--count", type=int, default=10, help="Seconds before auto crash")
    args = parser.parse_args()

    if args.kill:
        # start crash thread
        threading.Thread(target=auto_crash, args=(args.count,), daemon=True).start()

    #lamma ne run python server.py 50051, el server.py de argv[0] wel 50051 de argv[1]
    serve(args.port)
