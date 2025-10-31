#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

#include "parallelFunction.h"  
#include "sequentialFunction.h"
using namespace cv;
using namespace std;

int main(){

    double T_S = sequentialFunction();

    vector<double> T_Ps = parallelFunction();

    vector<int> threads = {1, 2, 4, 8};

    cout << "Threads (p) | T_P (s) | Speedup (S = T_S / T_P) | Efficiency (E = S / p)" << endl;
    cout << "-------------|---------|-------------------------|---------------------" << endl;
        
    for (size_t i = 0; i < T_Ps.size(); ++i) {
            double T_P = T_Ps[i];
            double S = T_S / T_P;
            double E = S / threads[i];
            
            cout << setw(11) << threads[i] << " | " << setw(7) << T_P << " | " << setw(23) << S << " | " << setw(19) << E << endl;
    }
        
    cout << "\nCalculations complete!" << endl;

}

