#include "pytorch_gds_integration.h"

/*
 * Placeholder for the real GDS backend. Replace these implementations
 * with CUDA/cuFile-backed logic when ENABLE_GDS=ON.
 */

int gds_is_available(void) {
    return 0;
}

int gds_init(void) {
    return -1;
}

const char* gds_get_last_error(void) {
    return "GDS backend not implemented";
}

GDSDataLoader* gds_loader_init(const char* nvme_path, size_t buffer_size, int batch_size) {
    (void)nvme_path;
    (void)buffer_size;
    (void)batch_size;
    return NULL;
}

void gds_loader_destroy(GDSDataLoader* loader) {
    (void)loader;
}

int gds_read_batch(GDSDataLoader* loader, int batch_idx, void** gpu_data) {
    (void)loader;
    (void)batch_idx;
    (void)gpu_data;
    return -1;
}

int gds_read_batch_async(GDSDataLoader* loader, int batch_idx, void** gpu_data, void* stream) {
    (void)loader;
    (void)batch_idx;
    (void)gpu_data;
    (void)stream;
    return -1;
}

int gds_prefetch_batch(GDSDataLoader* loader, int next_batch_idx) {
    (void)loader;
    (void)next_batch_idx;
    return -1;
}

void gds_print_stats(GDSDataLoader* loader) {
    (void)loader;
}

void gds_get_stats(GDSDataLoader* loader, GDSStats* stats) {
    (void)loader;
    if (!stats) {
        return;
    }
    stats->avg_throughput_gbps = 0.0;
    stats->avg_latency_ms = 0.0;
    stats->total_bytes = 0;
    stats->total_batches = 0;
}

void gds_reset_stats(GDSDataLoader* loader) {
    (void)loader;
}

void gds_shutdown(void) {
}
