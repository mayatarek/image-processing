#include "CacheSensitiveTest.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace cv;
using namespace std;
using namespace chrono;

void cacheSensitiveTest(const Mat& img) {


    int rows = img.rows;
    int cols = img.cols;

    volatile long long sink = 0; // prevents optimization

    // GOOD LOCALITY
    auto start1 = high_resolution_clock::now();
    for (int r = 0; r < 50; r++) { 
        for (int i = 0; i < rows; i++) {
            const uchar* row_ptr = img.ptr<uchar>(i);
            for (int j = 0; j < cols; j++) {
                sink += row_ptr[j];
            }
        }
    }
    auto end1 = high_resolution_clock::now();
    double t1 = duration<double>(end1 - start1).count();


    // BAD LOCALITY
    auto start2 = high_resolution_clock::now();
    for (int r = 0; r < 50; r++) {
        for (int j = 0; j < cols; j++) {
            for (int i = 0; i < rows; i++) {
                sink += img.at<uchar>(i, j); 
            }
        }
    }
    auto end2 = high_resolution_clock::now();
    double t2 = duration<double>(end2 - start2).count();

    cout << "\n--- Cache Sensitivity Test ---\n";
    cout << "Row-major time:   " << t1 << " s\n";
    cout << "Column-major time: " << t2 << " s\n";
    cout<< endl;
}

