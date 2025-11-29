#include <mpi.h>
#include <opencv2/opencv.hpp> //image loading and saving
#include <iostream>
#include <vector> //to store pixels
#include <cmath> //use sqrt
using namespace cv; //namespace 
using namespace std;
#include <fstream> 

int main(int argc, char** argv) {
    // initialize MPI
    MPI_Init(&argc, &argv); // Initialize MPI
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //id
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); //total number of processes

    Mat img; //matrix to hold the image
    int rows=0, cols=0;
    //only rank 0 loads the image and broadcasts size
    if (rank==0) {  //rank 0 loads the image
        img = imread("../data/cat.jpeg", IMREAD_GRAYSCALE); //load img & convert to grayscale
        if (img.empty()) { //handel if image not loaded
            cerr << "could not open image :(" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1); // stop all ranks if loading fails
        }
        resize(img, img, Size(4000,4000)); //resize for consistent testing
        rows = img.rows; //hight of image
        cols = img.cols; //width of image
        cout<<"Rank 0: image "<<rows<<" x "<<cols<<"\n"; // print image size
    }

   // Broadcast image size to all ranks
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD); //broadcast rows from ranl 0 to all ranks
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD); //broadcast cols from rank 0 to all ranks

   // divide work among ranks
    vector<int> counts(nprocs), displs(nprocs); //count nuber of processes (stores how many pixels each rank gets), displs (stores where each rank's data starts in the overall array)
    int base = rows / nprocs; //base number of rows per process
    int rem  = rows % nprocs; // if not divisible, some ranks get one extra row
    for (int r=0; r<nprocs; ++r) { // if among extra rows, first 'rem' ranks get one extra row
        counts[r] = (base + (r < rem ? 1 : 0)) * cols; // multipy by cols to get number of pixels
    }
    displs[0]=0; // first rank starts at 0
    for (int r=1; r<nprocs; ++r) displs[r] = displs[r-1] + counts[r-1]; // rank r starts after all previous ranks' data

    //local rows and extra halo rows
    int local_rows = counts[rank] / cols; //number of rows for this rank (each process knows how many rows it is responsible for)
  
    vector<unsigned char> local_buf((local_rows + 2) * cols, 0);  //halo rows (extra top and bottom row) to boundry data from neighboring ranks
   
    vector<unsigned char> recvbuf(local_rows * cols); //buffer to receive scattered data (has rows assigned to this rank)
    //scatter image data to all ranks
    MPI_Scatterv(rank==0 ? img.data : nullptr, counts.data(), displs.data(), MPI_UNSIGNED_CHAR, //scatter data from rank 0 to all ranks
                 recvbuf.data(), local_rows * cols, MPI_UNSIGNED_CHAR,
                 0, MPI_COMM_WORLD);

   
    for (int r=0; r<local_rows; ++r) { //copy received data into local_buf with offset for halo rows (gets the rows data and adds to local buffer withe extra halo rows)
        memcpy(&local_buf[(r+1)*cols], &recvbuf[r*cols], cols * sizeof(unsigned char));
    }

   
    vector<unsigned char> local_edges(local_rows * cols, 0); //buffer to hold edge-detected results for this rank

   // Sobel operator kernels
    int Gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    int Gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    int T = 30; //threshold


    MPI_Barrier(MPI_COMM_WORLD); // wait for all ranks to be ready
    double tstart = MPI_Wtime(); //start timing

    // Non-blocking halo exchange //  
    MPI_Request reqs[4]; // max 4 requests (2 sends, 2 recvs)
    int reqcount = 0; //num of requests
    if (rank > 0) { // asks ranks above
        MPI_Irecv(&local_buf[0*cols], cols, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD, &reqs[reqcount++]); // asks the rank above to send its bottom row to this rank's top halo row
        MPI_Isend(&local_buf[1*cols], cols, MPI_UNSIGNED_CHAR, rank-1, 1, MPI_COMM_WORLD, &reqs[reqcount++]); //asks the rank bellow to send its top row to this rank's bottom halo row
    }
    if (rank < nprocs-1) { // asking ranks below
        MPI_Irecv(&local_buf[(local_rows+1)*cols], cols, MPI_UNSIGNED_CHAR, rank+1, 1, MPI_COMM_WORLD, &reqs[reqcount++]); //asks the rank below to send its top row to this rank's bottom halo row
        MPI_Isend(&local_buf[local_rows*cols], cols, MPI_UNSIGNED_CHAR, rank+1, 0, MPI_COMM_WORLD, &reqs[reqcount++]); //asks the rank above to send its bottom row to this rank's top halo row
    }
    int start_r = 2; // skop first row 
    int end_r   = local_rows - 1; // skip last row
    if (local_rows <= 2) { start_r = 1; end_r = local_rows; } // skip edges if only 2 rows because we need neighbors
    // compute edges for inner rows (not halo rows)
    for (int currentr = start_r; currentr <= end_r; ++currentr) { //
        for (int currentc = 1; currentc < cols-1; ++currentc) {
            int sumx = 0, sumy = 0;
            for (int m=-1; m<=1; ++m) for (int n=-1; n<=1; ++n) {
                int pixel = local_buf[(currentr+m)*cols + (currentc+n)];
                sumx += pixel * Gx[m+1][n+1];
                sumy += pixel * Gy[m+1][n+1];
            } // look at neighboring and compute horizontal and verical sums
            double mag = sqrt((double)sumx*sumx + (double)sumy*sumy); // compute magnitude (edge strength)
            int out_r = currentr - 1; // 
            if (mag > T) local_edges[out_r*cols + currentc] = 255; // if strong enough edge, mark as edge 255 
            else local_edges[out_r*cols + currentc] = 0; //else mark as 0 backhground
        }
    }

   // wait for halo exchanges to complete before processing halo rows
    if (reqcount > 0) MPI_Waitall(reqcount, reqs, MPI_STATUSES_IGNORE); // wait for all halo exchanges to complete
    for (int c=1; c<cols-1; ++c) { //compute top halo row
        int lr = 1; // first row of local_buf
        int sumx=0,sumy=0;
        for (int m=-1; m<=1; ++m) for (int n=-1; n<=1; ++n) { // look at neighboring and compute horizontal and verical sums edges
            int pixel = local_buf[(lr+m)*cols + (c+n)];
            sumx += pixel * Gx[m+1][n+1];
            sumy += pixel * Gy[m+1][n+1];
        }
        double mag = sqrt((double)sumx*sumx + (double)sumy*sumy); // compute magnitude (edge strength)
        int out_r = lr - 1; 
        if (mag > T) local_edges[out_r*cols + c] = 255; else local_edges[out_r*cols + c] = 0; // if strong enough edge, mark as edge 255 else mark as 0 backhground
    }

    for (int c=1; c<cols-1; ++c) { //compute bottom halo row
        int lr = local_rows; // last row of local_buf
        int sumx=0,sumy=0;
        for (int m=-1; m<=1; ++m) for (int n=-1; n<=1; ++n) { // look at neighboring and compute horizontal and verical sums edges
            int pixel = local_buf[(lr+m)*cols + (c+n)];
            sumx += pixel * Gx[m+1][n+1];
            sumy += pixel * Gy[m+1][n+1];
        }
        double mag = sqrt((double)sumx*sumx + (double)sumy*sumy); // compute magnitude (edge strength)
        int out_r = lr - 1;
        if (mag > T) local_edges[out_r*cols + c] = 255; else local_edges[out_r*cols + c] = 0; // if strong enough edge, mark as edge 255 else mark as 0 backhground
    }

    // Finish timing
    MPI_Barrier(MPI_COMM_WORLD); // wait for all ranks to finish
    double tend = MPI_Wtime(); //end timing
    double elapsed = tend - tstart; //elapsed time


// for strong scaling analysis, save times to txt file so automate calcultions 
    if (rank == 0) {
        
        cout << "Processes: " << nprocs << ", Time: " << elapsed << " s\n";   
        
        // save to file
        ofstream outfile("mpi_times.txt", ios::app); // append mode
        if (outfile.is_open()) {
            outfile << nprocs << " " << elapsed << "\n";
            outfile.close();
        }
    }




    vector<int> recvcounts(nprocs), recvdispls(nprocs); // prepare to gather results
    for (int r=0;r<nprocs;++r) { // gather counts
        recvcounts[r] = counts[r]; // each rank's data size
    }
    recvdispls[0] = 0; // first rank starts at 0
    for (int r=1;r<nprocs;++r) recvdispls[r] = recvdispls[r-1] + recvcounts[r-1]; // each rank starts after all previous ranks' data
    vector<unsigned char> final_buf; // final buffer to hold gathered results
    if (rank==0) final_buf.resize(rows * cols); // only rank 0 needs to hold the final image
   // gather results to rank 0
    MPI_Gatherv(local_edges.data(), local_rows * cols, MPI_UNSIGNED_CHAR, //gather results to rank 0 all ranks send their local edge data
                rank==0 ? final_buf.data() : nullptr, recvcounts.data(), recvdispls.data(), MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD); 
    // rank 0 saves the final image
    if (rank == 0) {
        Mat edges(rows, cols, CV_8UC1, final_buf.data()); //create opencv image for final gathered data
        imwrite("mpi_edges.jpg", edges);
        cout<<"Saved mpi_edges.jpg\n";
    }

    MPI_Finalize(); //finalize MPI
    return 0;
}
