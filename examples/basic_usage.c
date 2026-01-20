/**
 * @file basic_usage.c
 * @brief Basic usage example for CV algorithms
 * 
 * This example demonstrates:
 * - Loading an image
 * - Applying edge detection
 * - Detecting corners
 * - Saving results
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/cv_algorithms.h"

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

int main(int argc, char* argv[]) {
    const char* input_path = "input.raw";
    const char* output_edges = "edges.raw";
    const char* output_corners = "corners.txt";

    if (argc > 1) {
        input_path = argv[1];
    }

    printf("=== ComputerVisionEmbedded Basic Usage Example ===\n\n");

    /* Load image */
    printf("[1] Loading image: %s (%dx%d)\n", input_path, IMAGE_WIDTH, IMAGE_HEIGHT);
    Image* img = cv_image_load_raw(input_path, IMAGE_WIDTH, IMAGE_HEIGHT);
    
    if (!img) {
        /* Create a test image if file doesn't exist */
        printf("    Creating synthetic test image...\n");
        img = cv_image_create(IMAGE_WIDTH, IMAGE_HEIGHT);
        if (!img) {
            fprintf(stderr, "Failed to create test image\n");
            return 1;
        }
        
        /* Fill with gradient + noise pattern */
        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                int val = (x + y) % 256;
                /* Add some edges */
                if (x % 100 < 5 || y % 100 < 5) {
                    val = 255;
                }
                img->data[y * IMAGE_WIDTH + x] = (uint8_t)val;
            }
        }
    }

    /* Apply edge detection */
    printf("\n[2] Applying Sobel edge detection...\n");
    Image* edges = cv_sobel_edge_detection(img);
    if (edges) {
        cv_image_save_raw(edges, output_edges);
        printf("    Saved edges to: %s\n", output_edges);
    }

    /* Detect corners */
    printf("\n[3] Detecting FAST corners...\n");
    int num_corners = 0;
    Corner* corners = cv_fast_corners(img, &num_corners);
    
    if (corners && num_corners > 0) {
        printf("    Found %d corners\n", num_corners);
        
        /* Save corners to file */
        FILE* fp = fopen(output_corners, "w");
        if (fp) {
            fprintf(fp, "# FAST Corners: x, y, strength\n");
            for (int i = 0; i < num_corners && i < 100; i++) {
                fprintf(fp, "%.1f, %.1f, %.2f\n", 
                        corners[i].x, corners[i].y, corners[i].strength);
            }
            fclose(fp);
            printf("    Saved corners to: %s\n", output_corners);
        }
        
        /* Print first few corners */
        printf("\n    First 5 corners:\n");
        for (int i = 0; i < 5 && i < num_corners; i++) {
            printf("      [%d] (%.1f, %.1f) strength=%.2f\n",
                   i, corners[i].x, corners[i].y, corners[i].strength);
        }
    }

    /* Cleanup */
    printf("\n[4] Cleaning up...\n");
    cv_image_free(img);
    cv_image_free(edges);
    free(corners);

    printf("\n=== Example complete ===\n");
    return 0;
}
