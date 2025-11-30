rank_time = {} 

with open("mpi_times.txt", "r") as f:
    for line in f:
        rank, time = line.split()
        rank_time[int(rank)] = float(time)  # this overwrites duplicates --> keep the latest 
        
        
#needed for calculations
sorted_ranks = sorted(rank_time.keys())
T1 = rank_time[1]  # baseline is rank 1

print(f"{'Ranks':>5} {'Time(s)':>10} {'Speedup':>10} {'Efficiency(%)':>15}")

# Compute speedup and efficiency 
for r in sorted_ranks:
    t = rank_time[r]
    speedup = T1 / t
    efficiency = speedup / r * 100
    print(f"{r:>5} {t:>10.5f} {speedup:>10.2f} {efficiency:>15.2f}")