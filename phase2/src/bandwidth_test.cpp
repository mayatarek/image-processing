// here we test bandwidth with large meassges 
#include <mpi.h>
#include <iostream>
using namespace std;
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (nprocs < 2) {
        if(rank==0) cout << "at least 2 processes\n";
        MPI_Finalize();
        return 0;
    }

    const int msg_size = 10 * 1024 * 1024; //10 MB message size for bandwidth test
    vector<char> send_buf(msg_size, 'a'); //buffer to send
    vector<char> recv_buf(msg_size); //buffer to receive

    const int iterations = 100; //number of iterations for averaging
    MPI_Barrier(MPI_COMM_WORLD); // so all ranks start together
    double tstart = MPI_Wtime(); //start time

    for (int i = 0; i < iterations; ++i) {
        if (rank == 0) {
            MPI_Send(send_buf.data(), msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD); //rank 0 sends
            MPI_Recv(recv_buf.data(), msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //rank 0 receives 
        } else if (rank == 1) {
            MPI_Recv(recv_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //rank 1 receives 
            MPI_Send(send_buf.data(), msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD); //rank 1 sends 
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); // wait for all ranks to finish
    double tend = MPI_Wtime(); //end time

    if (rank==0){

        double round_trip = (tend - tstart) / iterations; 
        double latency = round_trip / 2.0; 
        double bandwidth = (msg_size / latency) / 1e6; // MB/s
        cout << "average bandwidth for " << msg_size << " byte message: " << bandwidth << " MB/s\n";
    }
    MPI_Finalize();
    return 0;
}