import matplotlib.pyplot as plt

threads = [1, 2, 4, 8]
speedup = [1.01218, 2.03164, 3.96858, 5.25978]
efficiency = [1.01218, 1.01582, 0.992145, 0.657473]

plt.figure(figsize=(10,4))

# Speedup 
plt.subplot(1,2,1)
plt.plot(threads, speedup, marker='o', label='Observed')
plt.plot(threads, threads, '--', label='Ideal')  # dashed line for ideal linear speedup
plt.title('Speedup vs Threads')
plt.xlabel('Threads')
plt.ylabel('Speedup')
plt.legend()
plt.grid(True)

# Efficiency 
plt.subplot(1,2,2)
plt.plot(threads, efficiency, marker='o', color='orange')
plt.title('Efficiency vs Threads')
plt.xlabel('Threads')
plt.ylabel('Efficiency')
plt.grid(True)

plt.tight_layout()
plt.show()