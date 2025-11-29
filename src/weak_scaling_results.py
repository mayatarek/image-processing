rank_time = {}
rank_size = {}

with open("mpi_times_weak.txt", "r") as f:
    for line in f:
        parts = line.split()
        rank = int(parts[0])
        time = float(parts[1])
        size = parts[2]  # "rowsxcols"
        rank_time[rank] = time      
        rank_size[rank] = size      #  image size


sorted_ranks = sorted(rank_time.keys())

print(f"{'Ranks':>5} {'Time(s)':>10}  {'Image Size':>15}")
for r in sorted_ranks:
    t = rank_time[r]
    sz = rank_size[r]
    print(f"{r:>5} {t:>10.5f} {sz:>15}")

import matplotlib.pyplot as plt
ranks = sorted_ranks
times = [rank_time[r] for r in ranks]
plt.figure()
plt.plot(ranks, times, marker='o')

ideal_time = times[0]
plt.axhline(ideal_time, linestyle='--', label='Ideal Weak Scaling')

plt.xlabel('Number of Processes (Ranks)')
plt.ylabel('Execution Time (s)')
plt.title('Weak Scaling: Execution Time vs Number of Processes')
plt.xticks(ranks)
plt.grid(True)
plt.savefig('weak_scaling_plot.png')
plt.show()