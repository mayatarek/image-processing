# image-processing
## Parallel Edge Detection using the Sobel Operator
> The main idea behind the Sobel Operator is to find the gradient (rate and direction of change in pixel brightness).
> High gradient: usually indicates an edge, as it means a sharp change in brightness

---
## Logic:
-  We can represent each pixel in a grey-scale image as a 3x3 matrix where each pixel is the middle element and the 8 surrounding pixels are the 8 surrounding elements
- To process an image properly (find the edges), we will use 2 matrices, one for vertical processing and the other for horizontal. these metrices are known as the Sobel Kernels and they highlight changes in brightness
- Horizontal matrix Gx is [[-1, 0, 1], [-2,0,2], [-1,0,1]] while vertical matrix Gy is [[-1,-2,-1],[0,0,0],[1,2,1]]
- If we multiply each pixel and its neighbors by each of Gx and Gy, and then summing each of the outputs, we can get 2 numbers representing the change intensity horizontally and vertically
- By calculating the Euclidean distance "G" (the root of the sum of Gx and Gy each squared), we can get the "average" of the vertical and horizontal processing results
- The resulting vertical and horizontal intensity will be compared to T (threshold) - since each pixel can be from 0-255, the threshold of change can be 256/2 or 128
- If G > T: It is considered an edge pixel

---
### Compile and run main code
cd src
g++ -O2 -fopenmp -o main main.cpp parallel.cpp sequential.cpp cacheTest.cpp $(pkg-config --cflags --libs opencv4)
./main


### Get cache test data
perf stat -e cache-misses,cache-references ./main
