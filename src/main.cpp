#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

#include "parallelFunction.h"  
#include "sequentialFunction.h"
using namespace cv;
using namespace std;

int main(){

int T_S = sequentialFunction();

vector<double> T_Ps = parallelFunction();

vector<int> threads = {1, 2, 4, 8};




}

