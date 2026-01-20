# API Reference

This document provides the complete API reference for the ComputerVisionEmbedded library.

## 📦 Headers

| Header | Description |
|--------|-------------|
| `cv_algorithms.h` | Core CV functions |
| `pytorch_gds_integration.h` | GDS data loader |

---

## 🖼️ Image Structure

```c
typedef struct {
    uint32_t width;      // Image width in pixels
    uint32_t height;     // Image height in pixels
    uint8_t* data;       // Pixel data (row-major, single channel)
} Image;
```

---

## 🔍 Edge Detection

### cv_sobel_edge_detection

Performs Sobel edge detection on an input image.

```c
Image* cv_sobel_edge_detection(Image* input);
```

**Parameters**:

- `input`: Pointer to the input image

**Returns**:

- Pointer to a new Image containing edge magnitudes
- `NULL` on error

**Notes**:

- Caller is responsible for freeing the returned image with `cv_image_free()`
- Output values are normalized to 0-255

---

## 📍 Feature Detection

### cv_fast_corners

Detects corner keypoints using the FAST algorithm.

```c
typedef struct {
    float x, y;       // Corner coordinates
    float strength;   // Corner response strength
} Corner;

Corner* cv_fast_corners(Image* input, int* num_corners);
```

**Parameters**:

- `input`: Pointer to the input image
- `num_corners`: Output parameter for the number of detected corners

**Returns**:

- Array of Corner structures
- `NULL` on error (num_corners set to 0)

**Notes**:

- Caller is responsible for freeing the returned array with `free()`

---

## 🎨 Image Processing

### cv_normalize_image

Normalizes image pixel values to a specified range.

```c
void cv_normalize_image(Image* img, float min_val, float max_val);
```

**Parameters**:

- `img`: Pointer to the image (modified in-place)
- `min_val`: Target minimum value
- `max_val`: Target maximum value

---

### cv_image_free

Frees memory allocated for an Image structure.

```c
void cv_image_free(Image* img);
```

**Parameters**:

- `img`: Pointer to the image to free

---

## 💾 GDS Data Loader

### GDSDataLoader Structure

```c
typedef struct {
    CUdeviceptr gpu_buffer;   // GPU memory buffer
    size_t buffer_size;       // Buffer size in bytes
    const char* nvme_path;    // Path to NVMe file
    int batch_size;           // Batch size for loading
} GDSDataLoader;
```

---

### gds_loader_init

Initializes a GDS data loader for direct GPU-to-NVMe transfers.

```c
GDSDataLoader* gds_loader_init(const char* nvme_path, 
                                size_t buffer_size, 
                                int batch_size);
```

**Parameters**:

- `nvme_path`: Path to the data file on NVMe storage
- `buffer_size`: Size of the GPU buffer in bytes
- `batch_size`: Number of samples per batch

**Returns**:

- Pointer to initialized GDSDataLoader
- `NULL` on error

---

### gds_read_batch

Reads a batch of data directly from NVMe to GPU memory.

```c
int gds_read_batch(GDSDataLoader* loader, int batch_idx, void** gpu_data);
```

**Parameters**:

- `loader`: Pointer to the GDS loader
- `batch_idx`: Index of the batch to read
- `gpu_data`: Output pointer to GPU data

**Returns**:

- `0` on success
- `-1` on error

---

### gds_print_stats

Prints loader configuration and statistics.

```c
void gds_print_stats(GDSDataLoader* loader);
```

**Parameters**:

- `loader`: Pointer to the GDS loader

---

### gds_loader_destroy

Frees all resources associated with a GDS loader.

```c
void gds_loader_destroy(GDSDataLoader* loader);
```

**Parameters**:

- `loader`: Pointer to the GDS loader to destroy

---

## 🔧 Error Handling

All functions that can fail return `NULL` or `-1`. Check return values and use appropriate error handling:

```c
Image* edges = cv_sobel_edge_detection(input);
if (edges == NULL) {
    fprintf(stderr, "Edge detection failed\n");
    return -1;
}
```

---

## 📚 Related Documentation

- [Algorithms](algorithms.md)
- [GDS Setup Guide](gds-setup.md)
- [Architecture](architecture.md)
