ranks = []
times = []

with open("mpi_times.txt", "r") as f:
    for line in f:
        rank, time = line.split()
        ranks.append(int(rank))
        times.append(float(time))

#needed for calculations
T1 = times[0] #from mpi_times.txt, time for 1 rank


print(f"{'Ranks':>5} {'Time(s)':>10} {'Speedup':>10} {'Efficiency(%)':>15}")

# Compute speedup and efficiency 
for i, t in enumerate(times):
    speedup = T1 / t
    efficiency = (speedup / ranks[i]) * 100
    print(f"{ranks[i]:>5} {t:>10.5f} {speedup:>10.2f} {efficiency:>15.2f}")
