# Performance Tuning Guide

This guide covers performance optimization strategies for GPU-Direct Storage and computer vision workloads.

## 🎯 Overview

Performance in this system depends on three key areas:

1. **Storage I/O**: NVMe and GDS configuration
2. **GPU Compute**: Kernel optimization and memory management
3. **System Configuration**: OS and driver tuning

---

## 💾 Storage I/O Optimization

### NVMe Configuration

#### I/O Scheduler

```bash
# Use 'none' for NVMe (no scheduling needed)
echo "none" | sudo tee /sys/block/nvme0n1/queue/scheduler

# Verify
cat /sys/block/nvme0n1/queue/scheduler
# Output: [none] mq-deadline kyber bfq
```

#### Queue Depth

```bash
# Increase queue depth for better throughput
echo 1024 | sudo tee /sys/block/nvme0n1/queue/nr_requests
```

### GDS Configuration

#### cufile.json Optimization

```json
{
  "fs": {
    "properties": {
      "max_pinned_memory": 17179869184,  // 16GB for large datasets
      "allow_compat_mode": false          // Disable for pure GDS
    }
  },
  "nvme": {
    "properties": {
      "poll_mode": 1,        // 1 for high throughput
      "num_io_queues": 8     // Match CPU cores
    }
  }
}
```

#### Transfer Size Guidelines

| Workload | Recommended Size | Notes |
|----------|------------------|-------|
| Small batches | 1-4 MB | Optimize for latency |
| Training data | 16-64 MB | Balance throughput/latency |
| Large datasets | 128+ MB | Maximum throughput |

---

## 🚀 GPU Optimization

### Memory Management

#### Pinned Memory

```c
// Use pinned memory for CPU-GPU transfers
cudaMallocHost(&host_ptr, size);  // Pinned (faster)
// vs
malloc(size);  // Pageable (slower)
```

#### Memory Pools

```c
// Reduce allocation overhead with memory pools
cudaMemPool_t mempool;
cudaDeviceGetDefaultMemPool(&mempool, 0);
cudaMemPoolSetAttribute(mempool, 
    cudaMemPoolAttrReleaseThreshold, &threshold);
```

### Kernel Optimization

#### Thread Configuration

```c
// Optimal block size for most GPUs
dim3 block(16, 16);  // 256 threads per block
dim3 grid((width + 15) / 16, (height + 15) / 16);
```

#### Occupancy

```c
// Calculate optimal configuration
int minGridSize, blockSize;
cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, 
                                    myKernel, 0, 0);
```

### Streams and Concurrency

```c
// Use multiple streams for overlapping operations
cudaStream_t streams[4];
for (int i = 0; i < 4; i++) {
    cudaStreamCreate(&streams[i]);
}

// Overlap compute with I/O
for (int batch = 0; batch < num_batches; batch++) {
    int s = batch % 4;
    gds_read_batch_async(loader, batch, &data, streams[s]);
    process_batch<<<grid, block, 0, streams[s]>>>(data);
}
```

---

## 🖥️ System Configuration

### CPU Settings

```bash
# Disable frequency scaling
sudo cpupower frequency-set -g performance

# Disable CPU idle states
sudo cpupower idle-set -D 0
```

### NUMA Optimization

```bash
# Check NUMA topology
nvidia-smi topo -m

# Bind process to correct NUMA node
numactl --cpunodebind=0 --membind=0 ./your_program
```

### File Descriptor Limits

```bash
# Increase limits
ulimit -n 65536

# Or permanently in /etc/security/limits.conf
* soft nofile 65536
* hard nofile 65536
```

### Huge Pages

```bash
# Enable huge pages for large allocations
echo 1024 | sudo tee /proc/sys/vm/nr_hugepages

# Use huge pages in application
mmap(NULL, size, PROT_READ|PROT_WRITE, 
     MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0);
```

---

## 📊 Profiling Tools

### NVIDIA Tools

```bash
# System profiler
nsys profile -o report ./your_program

# Kernel profiler
ncu --set full ./your_program

# GDS stats
/usr/local/cuda/gds/tools/gds_stats
```

### Linux Tools

```bash
# I/O profiling
sudo perf record -e block:* ./your_program
sudo perf report

# Storage latency
sudo blktrace -d /dev/nvme0n1 -o trace
blkparse -i trace
```

---

## 📈 Benchmarking

### Quick Benchmark Script

```bash
#!/bin/bash
# benchmark.sh

echo "=== GDS Benchmark ==="
for size in 1M 16M 64M 256M; do
    echo "Testing $size transfers..."
    /usr/local/cuda/gds/tools/gdsio -f /mnt/nvme/test.bin \
        -d 0 -s $size -i 100 -w 0 -x 0
done
```

### Expected Results

| GPU | NVMe | Config | Read | Write |
|-----|------|--------|------|-------|
| RTX 4090 | PCIe 4.0 | poll_mode=1 | 14 GB/s | 12 GB/s |
| A100 | PCIe 4.0 | poll_mode=1 | 24 GB/s | 20 GB/s |
| Jetson Orin | Built-in | poll_mode=0 | 3.5 GB/s | 2.8 GB/s |

---

## 📚 Related Documentation

- [GDS Setup Guide](gds-setup.md)
- [Architecture Overview](architecture.md)
- [API Reference](api.md)
