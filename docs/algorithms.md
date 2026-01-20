# Computer Vision Algorithms

This document describes the computer vision algorithms implemented in this project.

## 🎯 Overview

The CV algorithm suite provides optimized implementations for embedded and GPU-accelerated systems, with a focus on:

- **Performance**: Optimized for real-time processing
- **Memory Efficiency**: Minimal memory footprint for embedded devices
- **GPU Acceleration**: CUDA kernels for parallel processing
- **GDS Integration**: Direct GPU-to-storage for large datasets

## 📊 Algorithms

### Edge Detection

#### Sobel Operator

Detects edges by computing image gradients in X and Y directions.

```
Gx = [-1  0  1]    Gy = [-1 -2 -1]
     [-2  0  2]         [ 0  0  0]
     [-1  0  1]         [ 1  2  1]

Magnitude = sqrt(Gx² + Gy²)
Direction = atan2(Gy, Gx)
```

**Implementation**: `src/cv_algorithms/edge_detection.c`

**Usage**:

```c
#include "cv_algorithms.h"

Image* input = load_image("input.raw");
Image* edges = cv_sobel_edge_detection(input);
```

**Performance**:

| Resolution | CPU Time | GPU Time | Speedup |
|------------|----------|----------|---------|
| 640x480    | 12ms     | 0.8ms    | 15x     |
| 1920x1080  | 85ms     | 2.1ms    | 40x     |
| 4096x2160  | 340ms    | 6.5ms    | 52x     |

---

### Feature Detection

#### FAST Corners

Features from Accelerated Segment Test - detects corner keypoints.

```
For each pixel p with intensity Ip:
  1. Examine 16 pixels on Bresenham circle (radius 3)
  2. If N contiguous pixels are brighter/darker by threshold t
  3. Mark as corner
```

**Implementation**: `src/cv_algorithms/feature_extraction.c`

**Usage**:

```c
#include "cv_algorithms.h"

Image* input = load_image("input.raw");
int num_corners;
Corner* corners = cv_fast_corners(input, &num_corners);
```

**Parameters**:

| Parameter | Description | Default |
|-----------|-------------|---------|
| `threshold` | Intensity difference threshold | 20 |
| `n_continuous` | Number of contiguous pixels | 9 |
| `non_max_suppression` | Enable NMS | true |

---

### Image Processing

#### Normalization

Scales pixel values to a specified range.

```
output = (input - min) / (max - min) * (target_max - target_min) + target_min
```

**Implementation**: `src/cv_algorithms/image_processing.c`

**Usage**:

```c
#include "cv_algorithms.h"

Image* img = load_image("input.raw");
cv_normalize_image(img, 0.0f, 1.0f);  // Normalize to [0, 1]
```

---

## 🚀 GPU Kernels

All algorithms have CUDA implementations in `src/gpu_kernels/`:

```cpp
// GPU Sobel kernel (simplified)
__global__ void sobel_kernel(const uint8_t* input, float* output,
                             int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x > 0 && x < width-1 && y > 0 && y < height-1) {
        float gx = // compute horizontal gradient
        float gy = // compute vertical gradient
        output[y * width + x] = sqrtf(gx*gx + gy*gy);
    }
}
```

## 🔄 GDS Integration

For large image datasets, use GDS to load directly to GPU:

```c
#include "pytorch_gds_integration.h"

// Initialize GDS loader
GDSDataLoader* loader = gds_loader_init("/mnt/nvme/images.bin", 
                                        1024*1024*64,  // 64MB buffer
                                        32);           // batch size

// Read batch directly to GPU
void* gpu_data;
gds_read_batch(loader, batch_idx, &gpu_data);

// Process on GPU
cuda_process_images(gpu_data);
```

## 📚 Related Documentation

- [API Reference](api.md)
- [GDS Setup Guide](gds-setup.md)
- [Performance Tuning](performance-tuning.md)
