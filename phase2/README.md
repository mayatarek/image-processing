How to run?

mpirun -np 1 ./mpi
mpirun -np 2 ./mpi
mpirun -np 4 ./mpi
mpirun -np 8 ./mpi
mpirun --oversubscribe -np 16 ./mpi 

python3 strong_scaling_results.py

