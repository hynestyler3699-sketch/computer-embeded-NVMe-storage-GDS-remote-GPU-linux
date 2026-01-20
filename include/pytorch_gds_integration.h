#ifndef PYTORCH_GDS_INTEGRATION_H
#define PYTORCH_GDS_INTEGRATION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations for CUDA types when CUDA is not available */
#ifndef __CUDA_RUNTIME_H__
typedef unsigned long long CUdeviceptr;
#endif

/**
 * @brief GDS Data Loader configuration and state
 */
typedef struct {
    CUdeviceptr gpu_buffer;      /**< GPU memory buffer for data loading */
    size_t buffer_size;          /**< Size of GPU buffer in bytes */
    const char* nvme_path;       /**< Path to NVMe storage file */
    int batch_size;              /**< Number of samples per batch */
    
    /* Internal state */
    void* file_handle;           /**< cuFile handle (opaque) */
    size_t total_bytes_read;     /**< Total bytes read since init */
    size_t total_batches_read;   /**< Total batches read since init */
    double total_read_time_ms;   /**< Total read time in milliseconds */
} GDSDataLoader;

/**
 * @brief GDS statistics structure
 */
typedef struct {
    double avg_throughput_gbps;  /**< Average throughput in GB/s */
    double avg_latency_ms;       /**< Average latency per batch in ms */
    size_t total_bytes;          /**< Total bytes transferred */
    size_t total_batches;        /**< Total batches processed */
} GDSStats;

/* ============================================================================
 * Initialization and Cleanup
 * ============================================================================ */

/**
 * @brief Initialize GDS subsystem
 * @return 0 on success, -1 on failure
 * @note Must be called before any other GDS functions
 */
int gds_init(void);

/**
 * @brief Shutdown GDS subsystem
 */
void gds_shutdown(void);

/**
 * @brief Initialize a GDS data loader
 * @param nvme_path Path to the data file on NVMe storage
 * @param buffer_size Size of GPU buffer in bytes
 * @param batch_size Number of samples per batch
 * @return Pointer to initialized loader, or NULL on failure
 */
GDSDataLoader* gds_loader_init(const char* nvme_path, size_t buffer_size, int batch_size);

/**
 * @brief Destroy a GDS data loader and free resources
 * @param loader Pointer to loader to destroy
 */
void gds_loader_destroy(GDSDataLoader* loader);

/* ============================================================================
 * Data Loading
 * ============================================================================ */

/**
 * @brief Read a batch directly from NVMe to GPU memory
 * @param loader Pointer to GDS loader
 * @param batch_idx Index of batch to read
 * @param gpu_data Output: pointer to GPU data
 * @return 0 on success, -1 on failure
 */
int gds_read_batch(GDSDataLoader* loader, int batch_idx, void** gpu_data);

/**
 * @brief Read a batch asynchronously
 * @param loader Pointer to GDS loader
 * @param batch_idx Index of batch to read
 * @param gpu_data Output: pointer to GPU data
 * @param stream CUDA stream for async operation
 * @return 0 on success, -1 on failure
 */
int gds_read_batch_async(GDSDataLoader* loader, int batch_idx, void** gpu_data, void* stream);

/**
 * @brief Prefetch next batch for improved throughput
 * @param loader Pointer to GDS loader
 * @param next_batch_idx Index of next batch to prefetch
 * @return 0 on success, -1 on failure
 */
int gds_prefetch_batch(GDSDataLoader* loader, int next_batch_idx);

/* ============================================================================
 * Statistics and Debugging
 * ============================================================================ */

/**
 * @brief Print loader configuration and statistics
 * @param loader Pointer to GDS loader
 */
void gds_print_stats(GDSDataLoader* loader);

/**
 * @brief Get detailed statistics
 * @param loader Pointer to GDS loader
 * @param stats Output: statistics structure
 */
void gds_get_stats(GDSDataLoader* loader, GDSStats* stats);

/**
 * @brief Reset statistics counters
 * @param loader Pointer to GDS loader
 */
void gds_reset_stats(GDSDataLoader* loader);

/**
 * @brief Check if GDS is available and working
 * @return 1 if GDS is available, 0 otherwise
 */
int gds_is_available(void);

/**
 * @brief Get last error message
 * @return Pointer to error string (do not free)
 */
const char* gds_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif /* PYTORCH_GDS_INTEGRATION_H */
