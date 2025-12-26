import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# 1. Check if data exists
if not os.path.exists('metrics.csv'):
    print("ERROR: metrics.csv not found. Please run client.py first.")
    sys.exit()

# 2. Load data
try:
    data = pd.read_csv('metrics.csv')
    if data.empty:
        print("ERROR: metrics.csv is empty.")
        sys.exit()
except Exception as e:
    print(f"ERROR reading CSV: {e}")
    sys.exit()

print(f"Loaded {len(data)} rows. Generating annotated graphs...")

# 3. Prepare Data
start_time = data['Timestamp'].min()
data['TimeRelative'] = data['Timestamp'] - start_time

# --- AUTOMATIC ANNOTATION LOGIC ---
# Find the 2 highest latency spikes (The Failures)
failures = data.nlargest(2, 'Latency').sort_values('TimeRelative')

# --- GRAPH 1: Latency (Annotated) ---
plt.figure(figsize=(10, 6))
plt.plot(data['TimeRelative'], data['Latency'], label='Latency', color='blue')

# Draw P95 Line
p95 = data['Latency'].quantile(0.95)
plt.axhline(y=p95, color='red', linestyle='--', label=f'P95 ({p95:.3f}s)')

# Loop through the failures and draw arrows
for index, row in failures.iterrows():
    # Arrow pointing to the crash
    plt.annotate('Failure Injection', 
                 xy=(row['TimeRelative'], row['Latency']), 
                 xytext=(row['TimeRelative'] + 2, row['Latency']),
                 arrowprops=dict(facecolor='black', shrink=0.05),
                 fontsize=10, color='red', weight='bold')
    
    # Text marking recovery (roughly 2 seconds after spike)
    plt.text(row['TimeRelative'] + 1.5, p95 + 0.1, "Recovery \u2192", 
             color='green', fontsize=10, weight='bold')

plt.title('Latency vs Time (Annotated)')
plt.xlabel('Time (s)')
plt.ylabel('Latency (s)')
plt.legend()
plt.grid(True)
plt.savefig('graph_latency.png')
print("SUCCESS: Saved annotated graph_latency.png")

# --- GRAPH 2: Throughput (Annotated) ---
data['Second'] = data['TimeRelative'].astype(int)
throughput = data[data['Status'] == 'Success'].groupby('Second').size()

plt.figure(figsize=(10, 6))
plt.plot(throughput.index, throughput.values, label='Throughput', color='green')

# Annotate dips on Throughput graph based on the same failure times
for index, row in failures.iterrows():
    fail_time = row['TimeRelative']
    # Find the throughput value near this time
    try:
        y_val = throughput.loc[int(fail_time)]
        plt.annotate('Dip', 
                     xy=(fail_time, y_val), 
                     xytext=(fail_time + 2, y_val + 5),
                     arrowprops=dict(facecolor='black', shrink=0.05),
                     fontsize=10, color='red')
    except:
        pass # Safety skip if index missing

plt.title('Throughput vs Time (Annotated)')
plt.xlabel('Time (s)')
plt.ylabel('Req/Sec')
plt.legend()
plt.grid(True)
plt.savefig('graph_throughput.png')
print("SUCCESS: Saved annotated graph_throughput.png")
