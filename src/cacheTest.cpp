#include "CacheSensitiveTest.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace cv;
using namespace std;
using namespace chrono;

void cacheSensitiveTestS(const Mat& img) {


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



void cacheSensitiveTestP(const Mat& img) {

    vector<int> thread_counts = {1, 2, 4, 8};

    int rows = img.rows;
    int cols = img.cols;

    volatile long long sink = 0; // prevents optimization

    for (int current_thread_count : thread_counts) {

    // GOOD LOCALITY 34n we access kol adjacent byte after b3d
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


    // BAD LOCALITY 34n access by columns so w3e jump over whole rest of row to access next column
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

    cout << "\n--- Cache Sensitivity Test, "<< current_thread_count<<" Threads---\n";
    cout << "Row-major time (Good):   " << t1 << " s\n";
    cout << "Column-major time (Bad): " << t2 << " s\n";
    cout<< endl;
}
}

