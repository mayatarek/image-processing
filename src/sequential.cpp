#include "sequentialFunction.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace cv;
using namespace std;
using namespace chrono;


double sequentialFunction() {

    Mat img = imread("../data/cat.jpeg", IMREAD_GRAYSCALE);  //load img
    if (img.empty()) {
        cout << "Error: could not open image!" << endl;
        return -1;
    }
    else{
        cout << "Image loaded successfully" << endl;
    }
    
    int T = 128;
    int Gx[3][3] = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};
    int Gy[3][3] = {{-1, -2, -1},{0, 0, 0},{1, 2, 1}};

    Mat edges=Mat::zeros(img.size(), CV_8UC1);
    int rows=img.rows;
    int cols=img.cols;

    auto start = high_resolution_clock::now();

    //loop on every pixel except the edgemost picels
    for (int i=1;i<rows-1;i++) {
        for (int j=1;j<cols-1;j++) {
            int sum_x=0;
            int sum_y =0;

            //multiply each pixel and neighbors by Gx and Gy
            for (int m= -1; m<= 1; m++) {
                for (int n =-1; n<= 1; n++) {
                    int pixel = img.at<uchar>(i+m,j+n);
                    sum_x +=pixel*Gx[m+1][n+1];
                    sum_y += pixel*Gy[m+1][n+1];
                }
            }

            //check sorbel result
            double mag = sqrt(sum_x*sum_x + sum_y*sum_y);

            //if above threshold make it an edge else dont
            if (mag > T)
                edges.at<uchar>(i, j) = 255;
            else
                edges.at<uchar>(i, j) = 0;
        }
    }

    auto end = high_resolution_clock::now();  
    double T_S = duration<double, milli>(end - start).count() / 1000.0;  // In seconds

    cout << "T_S: " << T_S << " s" << endl;

    imshow("Original", img);
    imshow("Edges", edges);
    //save the edges image
    imwrite("edges.jpg", edges);
    cout << "Saved edges.jpg in current folder!" << endl;

    // waitKey(0);
    return T_S;
}


