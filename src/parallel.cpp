#include "parallelFunction.h"
#include "CacheSensitiveTest.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>   
#include <iomanip>   
#include <omp.h>




using namespace cv;
using namespace std;
using namespace chrono;

vector<double> parallelFunction() {

    vector<int> thread_counts = {1, 2, 4, 8};
    vector<double> T_Ps;

    Mat img = imread("../data/cat.jpeg", IMREAD_GRAYSCALE);  //load img & convert to grayscale
    if (img.empty()) {
        cout << "Error: could not open image!" << endl;
        return T_Ps;
    }
    else{
        cout << "Image loaded successfully" << endl;
    }
    
    int T = 30;
    int Gx[3][3] = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};
    int Gy[3][3] = {{-1, -2, -1},{0, 0, 0},{1, 2, 1}};

    resize(img, img, Size(4000, 4000));


    Mat edges=Mat::zeros(img.size(), CV_8UC1); // create an empty image for edges
    int rows=img.rows;
    int cols=img.cols;


   for (int current_thread_count : thread_counts) {
        Mat edges = Mat::zeros(img.size(), CV_8UC1);  // Reset edges for each run
        
        auto start = high_resolution_clock::now();

        omp_set_num_threads(current_thread_count);

            #pragma omp parallel for collapse(2) 
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
        double T_P = duration<double, milli>(end - start).count() / 1000.0;  // In seconds

        cout << "T_P: " << T_P<< " s for "<< current_thread_count<< " threads" << endl;
        
        T_Ps.push_back(T_P);

        imshow("Original", img);
        imshow("Edges", edges);
        //save the edges image
        string filename = "edges_" + to_string(current_thread_count) + ".jpg";
        imwrite(filename, edges);
        cout << "Saved "<<filename <<" in current folder!" << endl;

    }

    // waitKey(0);
    cacheSensitiveTest(img);
    return T_Ps;
}


