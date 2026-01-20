/**
 * @file gds_integration.c
 * @brief Example of GPU-Direct Storage integration
 * 
 * This example demonstrates:
 * - Initializing GDS
 * - Loading data directly to GPU
 * - Processing on GPU
 * - Performance measurement
 * 
 * Requires: CUDA, cuFile (GDS), nvidia-fs module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/pytorch_gds_integration.h"

#define BUFFER_SIZE (64 * 1024 * 1024)  /* 64 MB */
#define BATCH_SIZE 32
#define NUM_BATCHES 10

/**
 * @brief Get current time in milliseconds
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/**
 * @brief Simulate processing on GPU (placeholder)
 */
static void process_on_gpu(void* gpu_data, size_t size) {
    (void)gpu_data;
    (void)size;
    /* In a real implementation, this would launch CUDA kernels */
    printf("    Processing %zu bytes on GPU...\n", size);
}

int main(int argc, char* argv[]) {
    const char* data_path = "/mnt/nvme/training_data.bin";
    
    if (argc > 1) {
        data_path = argv[1];
    }

    printf("=== GPU-Direct Storage Integration Example ===\n\n");

    /* Check GDS availability */
    printf("[1] Checking GDS availability...\n");
    if (!gds_is_available()) {
        printf("    WARNING: GDS not available, falling back to standard I/O\n");
        printf("    To enable GDS:\n");
        printf("      1. Install nvidia-fs kernel module\n");
        printf("      2. Configure /etc/cufile.json\n");
        printf("      3. Ensure NVMe drive supports P2P\n\n");
        /* Continue with demo anyway */
    } else {
        printf("    GDS is available!\n");
    }

    /* Initialize GDS subsystem */
    printf("\n[2] Initializing GDS subsystem...\n");
    if (gds_init() != 0) {
        fprintf(stderr, "    Warning: GDS init failed: %s\n", gds_get_last_error());
        /* Continue for demo purposes */
    }

    /* Create data loader */
    printf("\n[3] Creating GDS data loader...\n");
    printf("    Path: %s\n", data_path);
    printf("    Buffer size: %d MB\n", BUFFER_SIZE / (1024 * 1024));
    printf("    Batch size: %d\n", BATCH_SIZE);

    GDSDataLoader* loader = gds_loader_init(data_path, BUFFER_SIZE, BATCH_SIZE);
    
    if (!loader) {
        fprintf(stderr, "    Warning: Loader init failed (file may not exist)\n");
        fprintf(stderr, "    Creating simulated demo...\n\n");
        
        /* Demo without actual GDS */
        printf("[DEMO MODE] Simulating GDS data loading...\n\n");
        
        double total_time = 0;
        double total_bytes = 0;
        
        for (int i = 0; i < NUM_BATCHES; i++) {
            double start = get_time_ms();
            
            /* Simulate read latency */
            struct timespec delay = {0, 5000000};  /* 5ms */
            nanosleep(&delay, NULL);
            
            double elapsed = get_time_ms() - start;
            total_time += elapsed;
            total_bytes += BUFFER_SIZE;
            
            printf("  Batch %d: %.2f ms (simulated)\n", i, elapsed);
        }
        
        printf("\n[DEMO] Results:\n");
        printf("  Total batches: %d\n", NUM_BATCHES);
        printf("  Total data: %.2f GB\n", total_bytes / 1e9);
        printf("  Total time: %.2f ms\n", total_time);
        printf("  Throughput: %.2f GB/s (simulated)\n", 
               (total_bytes / 1e9) / (total_time / 1000.0));
        
        gds_shutdown();
        printf("\n=== Demo complete ===\n");
        return 0;
    }

    /* Print loader stats */
    printf("\n[4] Loader configuration:\n");
    gds_print_stats(loader);

    /* Load batches and measure performance */
    printf("\n[5] Loading %d batches...\n", NUM_BATCHES);
    
    double start_time = get_time_ms();
    
    for (int batch = 0; batch < NUM_BATCHES; batch++) {
        void* gpu_data = NULL;
        double batch_start = get_time_ms();
        
        int result = gds_read_batch(loader, batch, &gpu_data);
        
        double batch_time = get_time_ms() - batch_start;
        
        if (result == 0) {
            printf("  Batch %d: %.2f ms, GPU ptr=%p\n", 
                   batch, batch_time, gpu_data);
            
            /* Process on GPU */
            process_on_gpu(gpu_data, BUFFER_SIZE);
        } else {
            printf("  Batch %d: FAILED\n", batch);
        }
    }
    
    double total_time = get_time_ms() - start_time;

    /* Print final statistics */
    printf("\n[6] Performance results:\n");
    GDSStats stats;
    gds_get_stats(loader, &stats);
    
    printf("  Total batches: %zu\n", stats.total_batches);
    printf("  Total data: %.2f GB\n", stats.total_bytes / 1e9);
    printf("  Total time: %.2f ms\n", total_time);
    printf("  Throughput: %.2f GB/s\n", stats.avg_throughput_gbps);
    printf("  Avg latency: %.2f ms/batch\n", stats.avg_latency_ms);

    /* Cleanup */
    printf("\n[7] Cleaning up...\n");
    gds_loader_destroy(loader);
    gds_shutdown();

    printf("\n=== Example complete ===\n");
    return 0;
}
