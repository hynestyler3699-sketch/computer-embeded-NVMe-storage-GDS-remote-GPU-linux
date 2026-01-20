/**
 * @file edge_detection.c
 * @brief Edge detection algorithms implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cv_algorithms.h"

/* Sobel kernels */
static const int sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

static const int sobel_y[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

/**
 * @brief Get pixel value with boundary checking
 */
static inline uint8_t get_pixel(const Image* img, int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= (int)img->width) x = img->width - 1;
    if (y >= (int)img->height) y = img->height - 1;
    return img->data[y * img->width + x];
}

Image* cv_sobel_edge_detection(Image* input) {
    if (!input || !input->data) {
        fprintf(stderr, "[CV] Error: Invalid input image\n");
        return NULL;
    }

    Image* output = cv_image_create(input->width, input->height);
    if (!output) {
        fprintf(stderr, "[CV] Error: Failed to allocate output image\n");
        return NULL;
    }

    printf("[CV] Sobel edge detection: %ux%u\n", input->width, input->height);

    for (uint32_t y = 0; y < input->height; y++) {
        for (uint32_t x = 0; x < input->width; x++) {
            int gx = 0, gy = 0;

            /* Apply Sobel kernels */
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = get_pixel(input, x + kx, y + ky);
                    gx += pixel * sobel_x[ky + 1][kx + 1];
                    gy += pixel * sobel_y[ky + 1][kx + 1];
                }
            }

            /* Compute magnitude */
            int magnitude = (int)sqrt((double)(gx * gx + gy * gy));
            
            /* Clamp to 0-255 */
            if (magnitude > 255) magnitude = 255;
            if (magnitude < 0) magnitude = 0;

            output->data[y * output->width + x] = (uint8_t)magnitude;
        }
    }

    printf("[CV] Edge detection complete\n");
    return output;
}

int cv_sobel_edge_detection_full(Image* input, Image* magnitude, float* direction) {
    if (!input || !input->data || !magnitude || !magnitude->data) {
        fprintf(stderr, "[CV] Error: Invalid input/output images\n");
        return -1;
    }

    if (magnitude->width != input->width || magnitude->height != input->height) {
        fprintf(stderr, "[CV] Error: Output image dimensions mismatch\n");
        return -1;
    }

    for (uint32_t y = 0; y < input->height; y++) {
        for (uint32_t x = 0; x < input->width; x++) {
            int gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = get_pixel(input, x + kx, y + ky);
                    gx += pixel * sobel_x[ky + 1][kx + 1];
                    gy += pixel * sobel_y[ky + 1][kx + 1];
                }
            }

            int mag = (int)sqrt((double)(gx * gx + gy * gy));
            if (mag > 255) mag = 255;

            size_t idx = y * input->width + x;
            magnitude->data[idx] = (uint8_t)mag;
            
            if (direction) {
                direction[idx] = atan2f((float)gy, (float)gx);
            }
        }
    }

    return 0;
}
