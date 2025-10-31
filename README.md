# image-processing
## Parallel Edge Detection using the Sobel Operator

Logic:
1- we can represent each pixel ina grey-scale image as a 3x3 matrix where each pixel is the midle element and the 8 surrounding pixels are the 8 surrounding elements
2- to process an image properly we will use 2 matrices, one for vertical processing anf the other for horizontal. these metrices are known as the Sobel Kernels
3- horizontal matrix Gx is [[-1, 0, 1], [-2,0,2], [-1,0,1]] while vertical matrix Gy is [[-1,-2,-1],[0,0,0],[1,2,1]]
4- if we multiply each pixel + neighbors by each of Gx and Gy, and then summing each of the outputs, we can get 2 numbers representing the change intensity horizontally and vertically
5- by calculating the eucladien distance (the root of the sum of Gx and Gy squared), we can get the "avergae" of the vertical and horizontal processing results
6- the resulting vertical and horizontal intensity will be compared to T (threshold) - since each pixel can be from 0-255, the threshold of change can be 256/2 or 128

### Complile and run sequential code
```
cd src
./sequential
```

g++ parallel.cpp -o parallel -fopenmp `pkg-config --cflags --libs opencv4`

### Complile and run parallel code
```
cd src
g++ parallel.cpp -o parallel -fopenmp `pkg-config --cflags --libs opencv4`
./parallel
```