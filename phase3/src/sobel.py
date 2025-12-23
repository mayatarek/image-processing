import cv2
import numpy as np
import time

#da nafs el sequential code men phase 1 bas in python
def sobel_edge_detection(image_bytes):
    np_img = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(np_img, cv2.IMREAD_GRAYSCALE)

    if img is None:
        raise ValueError("Error: could not open image!")
    print("Image loaded successfully")
    print("starting sobel aho...")
    T = 30
    Gx = np.array([[-1,0,1],[-2,0,2],[-1,0,1]])
    Gy = np.array([[-1,-2,-1],[0,0,0],[1,2,1]])
    img = cv2.resize(img, (100, 100))

    rows, cols = img.shape
    edges = np.zeros_like(img)

    start = time.time()
    print(f"done 1/{rows}")
#//loop on every pixel except the edgemost picels
    for i in range(1, rows-1):
        print(f"done {i}/{rows}")
        for j in range(1, cols-1):
            region = img[i-1:i+2, j-1:j+2]
            sum_x = np.sum(region * Gx)
            sum_y = np.sum(region * Gy)
            mag = np.sqrt(sum_x**2 + sum_y**2)

            edges[i, j] = 255 if mag > T else 0

    latency = time.time() - start

    _, encoded = cv2.imencode(".jpeg", edges)
    print(f"Image Edges done with latency {latency}")
    return encoded.tobytes(), latency