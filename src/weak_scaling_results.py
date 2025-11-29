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

