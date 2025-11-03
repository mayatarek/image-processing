#include "CacheSensitiveTest.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <omp.h>

using namespace cv;
using namespace std;
using namespace chrono;

void cacheSensitiveTestS(const Mat& img) {


    int rows = img.rows;
    int cols = img.cols;

    volatile long long sink = 0; 

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


// so for the good locality, we have a pointer for each row since its stored major. 
// And we use that pointer and just walk done the row to get each value. 
// However, for the bad locality, we don't have a pointer since it's not stored column major. 
// So we need to use img.at<uchar>(i, j); which explicitly tells us how many rows i to go
// down and how mnay columns j. Ofc this also takes more time since you need 
// to calc the i and j for each value, u dont move down the row like in the good locality.



void cacheSensitiveTestP(const Mat& img) {

    vector<int> thread_counts = {1, 2, 4, 8};

    int rows = img.rows;
    int cols = img.cols;

    volatile long long sink = 0; 

    for (int current_thread_count : thread_counts) {
        omp_set_num_threads(current_thread_count); 

    // GOOD LOCALITY
    auto start1 = high_resolution_clock::now();
    #pragma omp parallel for collapse(2) reduction(+:sink) 
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
    #pragma omp parallel for collapse(2)  
    for (int r = 0; r < 50; r++) {
        for (int j = 0; j < cols; j++) {
            for (int i = 0; i < rows; i++) {
                sink += img.at<uchar>(i, j); 
            }
        }
    }
    auto end2 = high_resolution_clock::now();
    double t2 = duration<double>(end2 - start2).count();  

    cout << "\n--- Cache Sensitivity Test, "<< current_thread_count<<" Threads---\n";
    cout << "Row-major time (Good):   " << t1 << " s\n";
    cout << "Column-major time (Bad): " << t2 << " s\n";
    cout<< endl;
}
}