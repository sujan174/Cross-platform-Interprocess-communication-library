import json
import matplotlib.pyplot as plt
import numpy as np

def load_results(filename):
    try:
        with open(filename, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"File {filename} not found.")
        return None

def compare_results():
    
    cross_ipc_results = load_results("cross_ipc_results.json")
    zeromq_results = load_results("zeromq_results.json")
    
    if not cross_ipc_results or not zeromq_results:
        print("Missing results files. Run both benchmarks first.")
        return
    
    # Print comparison
    print("\n=== Performance Comparison ===")
    print(f"Payload size: {cross_ipc_results['payload_size']} bytes")
    print(f"Number of requests: {cross_ipc_results['num_requests']}")
    print("\nMetric                  | Cross-IPC      | ZeroMQ         | Difference")
    print("------------------------|----------------|----------------|----------------")
    
    
    cross_ipc_mean = cross_ipc_results['mean_latency']
    zeromq_mean = zeromq_results['mean_latency']
    diff_mean = cross_ipc_mean - zeromq_mean
    diff_mean_pct = (diff_mean / zeromq_mean) * 100 if zeromq_mean != 0 else float('inf')
    print(f"Mean latency (ms)       | {cross_ipc_mean:.2f}          | {zeromq_mean:.2f}          | {diff_mean:.2f} ({diff_mean_pct:.2f}%)")
    
    
    cross_ipc_median = cross_ipc_results['median_latency']
    zeromq_median = zeromq_results['median_latency']
    diff_median = cross_ipc_median - zeromq_median
    diff_median_pct = (diff_median / zeromq_median) * 100 if zeromq_median != 0 else float('inf')
    print(f"Median latency (ms)     | {cross_ipc_median:.2f}          | {zeromq_median:.2f}          | {diff_median:.2f} ({diff_median_pct:.2f}%)")
    
    # Compare min latency
    cross_ipc_min = cross_ipc_results['min_latency']
    zeromq_min = zeromq_results['min_latency']
    diff_min = cross_ipc_min - zeromq_min
    diff_min_pct = (diff_min / zeromq_min) * 100 if zeromq_min != 0 else float('inf')
    print(f"Min latency (ms)        | {cross_ipc_min:.2f}          | {zeromq_min:.2f}          | {diff_min:.2f} ({diff_min_pct:.2f}%)")
    
    # Compare max latency
    cross_ipc_max = cross_ipc_results['max_latency']
    zeromq_max = zeromq_results['max_latency']
    diff_max = cross_ipc_max - zeromq_max
    diff_max_pct = (diff_max / zeromq_max) * 100 if zeromq_max != 0 else float('inf')
    print(f"Max latency (ms)        | {cross_ipc_max:.2f}          | {zeromq_max:.2f}          | {diff_max:.2f} ({diff_max_pct:.2f}%)")
    
    
    cross_ipc_rps = cross_ipc_results['requests_per_second']
    zeromq_rps = zeromq_results['requests_per_second']
    diff_rps = cross_ipc_rps - zeromq_rps
    diff_rps_pct = (diff_rps / zeromq_rps) * 100 if zeromq_rps != 0 else float('inf')
    print(f"Requests per second     | {cross_ipc_rps:.2f}          | {zeromq_rps:.2f}          | {diff_rps:.2f} ({diff_rps_pct:.2f}%)")
    
    # Create visualizations
    plt.figure(figsize=(15, 10))
    
    
    plt.subplot(2, 2, 1)
    plt.hist(cross_ipc_results['latencies'], alpha=0.5, bins=30, label='Cross-IPC')
    plt.hist(zeromq_results['latencies'], alpha=0.5, bins=30, label='ZeroMQ')
    plt.xlabel('Latency (ms)')
    plt.ylabel('Frequency')
    plt.title('Latency Distribution')
    plt.legend()
    
    
    plt.subplot(2, 2, 2)
    plt.boxplot([cross_ipc_results['latencies'], zeromq_results['latencies']], 
                labels=['Cross-IPC', 'ZeroMQ'])
    plt.ylabel('Latency (ms)')
    plt.title('Latency Comparison')
    
    # Bar chart of key metrics
    plt.subplot(2, 2, 3)
    metrics = ['mean_latency', 'median_latency', 'min_latency', 'max_latency']
    cross_ipc_values = [cross_ipc_results[m] for m in metrics]
    zeromq_values = [zeromq_results[m] for m in metrics]
    
    x = np.arange(len(metrics))
    width = 0.35
    
    plt.bar(x - width/2, cross_ipc_values, width, label='Cross-IPC')
    plt.bar(x + width/2, zeromq_values, width, label='ZeroMQ')
    
    plt.xlabel('Metric')
    plt.ylabel('Latency (ms)')
    plt.title('Latency Metrics Comparison')
    plt.xticks(x, ['Mean', 'Median', 'Min', 'Max'])
    plt.legend()
    
    
    plt.subplot(2, 2, 4)
    plt.bar(['Cross-IPC', 'ZeroMQ'], [cross_ipc_rps, zeromq_rps])
    plt.ylabel('Requests per second')
    plt.title('Throughput Comparison')
    
    plt.tight_layout()
    plt.savefig('benchmark_comparison.png')
    plt.close()
    
    print("\nVisualization saved to benchmark_comparison.png")

if __name__ == "__main__":
    compare_results() 