# System Architecture

This document provides an overview of the system architecture for GPU-Direct Storage integrated computer vision.

## 🎯 Overview

The system is designed as a layered architecture optimizing data flow from storage to GPU for high-performance CV workloads.

## 🏗️ Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   PyTorch       │  │   TensorFlow    │  │   Custom     │ │
│  │   Training      │  │   Inference     │  │   C/C++ App  │ │
│  └────────┬────────┘  └────────┬────────┘  └──────┬───────┘ │
│           │                    │                   │         │
│           └────────────────────┼───────────────────┘         │
│                                ▼                             │
├─────────────────────────────────────────────────────────────┤
│                    Python Bindings                           │
│  ┌─────────────────┐  ┌─────────────────┐                   │
│  │     kvikio      │  │   pycufile      │                   │
│  └────────┬────────┘  └────────┬────────┘                   │
│           └────────────────────┘                             │
│                       ▼                                      │
├─────────────────────────────────────────────────────────────┤
│                GPU-Direct Storage Layer                      │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                    cuFile API                            ││
│  │  cuFileRead() | cuFileWrite() | cuFileBufRegister()     ││
│  └────────────────────────┬─────────────────────────────────┘│
│                           ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                  nvidia-fs Module                        ││
│  │            (Kernel-level GDS driver)                     ││
│  └────────────────────────┬─────────────────────────────────┘│
│                           ▼                                  │
├─────────────────────────────────────────────────────────────┤
│               CV Algorithm Core (C/C++)                      │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────┐  │
│  │    Edge      │  │   Feature    │  │      Image        │  │
│  │  Detection   │  │  Extraction  │  │   Processing      │  │
│  └──────┬───────┘  └──────┬───────┘  └────────┬──────────┘  │
│         └──────────────────┼──────────────────┘              │
│                            ▼                                 │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                   CUDA Kernels                           ││
│  │      GPU-accelerated implementations                     ││
│  └─────────────────────────────────────────────────────────┘│
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                    Hardware Layer                            │
│  ┌─────────────────┐    P2P/DMA    ┌───────────────────────┐│
│  │                 │◄──────────────►│                       ││
│  │   NVIDIA GPU    │                │    NVMe Storage       ││
│  │   (RTX/A100/    │                │    (Local or NVMeoF)  ││
│  │    Jetson)      │                │                       ││
│  └─────────────────┘                └───────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

## 📦 Components

### 1. Application Layer

The top layer where user applications run:

- **PyTorch Training**: Deep learning model training with DataLoader
- **TensorFlow Inference**: Model serving and inference
- **Custom C/C++ Apps**: Direct integration with CV core

### 2. Python Bindings

Python interfaces for GDS functionality:

- **kvikio**: High-level Pythonic API (recommended)
- **pycufile**: Low-level cuFile bindings

```python
import kvikio
import cupy as cp

# Direct GPU read with kvikio
with kvikio.CuFile("/data/images.bin", "r") as f:
    gpu_array = cp.empty(shape, dtype=cp.float32)
    f.read(gpu_array)
```

### 3. GPU-Direct Storage Layer

The core GDS implementation:

- **cuFile API**: User-space library for GDS operations
- **nvidia-fs**: Kernel module enabling P2P transfers

### 4. CV Algorithm Core

Optimized computer vision implementations:

| Component | Description | Location |
|-----------|-------------|----------|
| Edge Detection | Sobel, Canny | `src/cv_algorithms/` |
| Feature Extraction | FAST, SIFT | `src/cv_algorithms/` |
| Image Processing | Resize, Normalize | `src/cv_algorithms/` |
| CUDA Kernels | GPU implementations | `src/gpu_kernels/` |

### 5. Hardware Layer

Physical components:

- **GPU**: NVIDIA GPU with GDS support
- **NVMe**: High-speed storage (local or NVMe-oF)
- **P2P/DMA**: Direct memory access path

## 🔄 Data Flow

### Traditional I/O (Without GDS)

```
NVMe → Kernel Buffer → User Buffer → Pinned Memory → GPU
        4KB pages      memcpy()       cudaMemcpy()
        
Latency: High (multiple copies)
CPU Usage: 80%+
```

### With GPU-Direct Storage

```
NVMe → GPU Memory (Direct P2P Transfer)

Latency: Low (single transfer)
CPU Usage: <10%
```

## 🌐 Network Storage (NVMe-oF)

For remote GPU access:

```
┌──────────────┐         ┌──────────────┐
│   Initiator  │  RDMA   │    Target    │
│   (Client)   │◄───────►│   (Server)   │
│   + GPU      │  NVMeoF │   + NVMe     │
└──────────────┘         └──────────────┘
```

Configuration in `/etc/nvme/`:

```
# discovery.conf
nqn=nvme-subsys
transport=rdma
traddr=192.168.1.100
trsvcid=4420
```

## 📁 Directory Structure

```
computer-embeded-NVMe-storage-GDS-remote-GPU-linux/
├── src/
│   ├── cv_algorithms/     # Core CV implementations
│   ├── gpu_kernels/       # CUDA kernel code
│   └── nvme_integration/  # NVMe/GDS helpers
├── include/               # Header files
├── examples/              # Usage examples
├── tests/                 # Test suites
├── docs/                  # Documentation
└── scripts/               # Helper scripts
```

## 📚 Related Documentation

- [GDS Setup Guide](gds-setup.md)
- [API Reference](api.md)
- [Performance Tuning](performance-tuning.md)
- [Algorithms](algorithms.md)
