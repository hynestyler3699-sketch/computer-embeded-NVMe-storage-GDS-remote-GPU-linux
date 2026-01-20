# NVIDIA GPUDirect Storage (GDS) Setup Guide

This guide covers setting up GPU-Direct Storage for direct GPU-to-NVMe data transfers on Linux systems.

## 🎯 Overview

GPUDirect Storage (GDS) enables direct memory access (DMA) between GPU memory and storage, bypassing the CPU and system memory entirely. This can provide up to **10x throughput improvement** for data-intensive workloads.

```
Traditional I/O:
  NVMe → CPU Memory → GPU Memory (2 copies, CPU involved)

With GDS:
  NVMe → GPU Memory (1 direct transfer, CPU free)
```

## 📋 Prerequisites

### Hardware Requirements

- **GPU**: NVIDIA GPU with compute capability 6.0+ (Pascal or newer)
  - Recommended: RTX A6000, A100, Jetson AGX Orin
- **NVMe**: PCIe 3.0+ NVMe SSD
  - Recommended: PCIe 4.0 for maximum throughput
- **Platform**: x86_64 or aarch64 (Jetson)

### Software Requirements

- **OS**: Linux kernel 5.4+
- **CUDA**: 11.4+
- **Driver**: NVIDIA driver 470+

## 🛠️ Installation

### Step 1: Install CUDA Toolkit

```bash
# Ubuntu/Debian
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install cuda-toolkit-12-3
```

### Step 2: Build nvidia-fs Kernel Module

```bash
# Download GDS package (or use the included gds-nvidia-fs-master.zip)
unzip gds-nvidia-fs-master.zip
cd gds-nvidia-fs-master

# Build the module
make

# Install
sudo make install

# Load the module
sudo modprobe nvidia-fs

# Verify
dmesg | grep nvidia-fs
lsmod | grep nvidia_fs
```

### Step 3: Configure cufile.json

Create/edit `/etc/cufile.json`:

```json
{
  "logging": {
    "type": "stderr",
    "level": "ERROR"
  },
  "fs": {
    "properties": {
      "allow_compat_mode": true,
      "max_pinned_memory": 67108864,
      "allow_posix_read": true,
      "allow_posix_write": true
    }
  },
  "nvme": {
    "properties": {
      "poll_mode": 0,
      "num_io_queues": 4
    }
  }
}
```

### Step 4: Verify Installation

```bash
# Run GDS check tool
/usr/local/cuda/gds/tools/gdscheck -p

# Expected output should show:
# - nvidia_fs module loaded
# - P2P capability enabled
# - Compatible NVMe devices
```

## 📊 Performance Tuning

### cufile.json Parameters

| Parameter | Description | Recommended |
|-----------|-------------|-------------|
| `max_pinned_memory` | Max GPU memory for cuFile | 64MB - 16GB |
| `poll_mode` | 0=interrupt, 1=poll | 0 for latency, 1 for throughput |
| `num_io_queues` | Number of I/O queues | 4-8 for multi-threaded |

### System Tuning

```bash
# Disable CPU frequency scaling
sudo cpupower frequency-set -g performance

# Set I/O scheduler
echo "none" | sudo tee /sys/block/nvme0n1/queue/scheduler

# Increase file descriptor limits
ulimit -n 65536
```

## 🔧 Troubleshooting

### Issue: "P2P not supported"

**Cause**: BIOS ACS (Access Control Services) is enabled

**Solution**:

```bash
# Check ACS status
sudo lspci -vvv | grep -i acs

# Disable ACS in BIOS or use kernel parameter:
# iommu=pt
```

### Issue: Module fails to load

**Cause**: Kernel headers mismatch

**Solution**:

```bash
# Install matching headers
uname -r
sudo apt install linux-headers-$(uname -r)

# Rebuild module
cd gds-nvidia-fs-master
make clean
make
sudo make install
```

### Issue: Low throughput

**Cause**: Suboptimal configuration

**Solution**:

1. Enable poll mode (`poll_mode: 1`)
2. Increase `num_io_queues`
3. Use larger transfer sizes (≥1MB)
4. Verify NVMe is on same NUMA node as GPU

## 📚 API Quick Reference

### cuFile API

```c
#include <cufile.h>

// Initialize
cuFileDriverOpen();

// Open file handle
cuFileHandle_t fh;
cuFileHandleOpen(&fh, "/path/to/file");

// Register GPU buffer
cuFileBufRegister(gpu_ptr, size, 0);

// Read directly to GPU
cuFileRead(fh, gpu_ptr, size, file_offset, 0);

// Write from GPU
cuFileWrite(fh, gpu_ptr, size, file_offset, 0);

// Cleanup
cuFileBufDeregister(gpu_ptr);
cuFileHandleClose(fh);
cuFileDriverClose();
```

### kvikio (Python)

```python
import kvikio

# Open file
f = kvikio.CuFile("/path/to/file", "r")

# Read to CuPy array
import cupy as cp
arr = cp.empty(1024, dtype=cp.float32)
f.read(arr)

# Close
f.close()
```

## 📈 Benchmarks

| Configuration | Read Throughput | Write Throughput | CPU Usage |
|---------------|-----------------|------------------|-----------|
| Standard I/O  | 1.4 GB/s        | 1.2 GB/s         | 80%       |
| GDS (1MB)     | 6.8 GB/s        | 5.5 GB/s         | 15%       |
| GDS (64MB)    | 12.5 GB/s       | 10.2 GB/s        | 5%        |

## 📚 Related Documentation

- [Architecture Overview](architecture.md)
- [Performance Tuning](performance-tuning.md)
- [API Reference](api.md)
- [Colab Integration](COLAB_SYNC.md)
