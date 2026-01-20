/**
 * @file feature_extraction.c
 * @brief Feature detection algorithms (FAST corners)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cv_algorithms.h"

/* Bresenham circle offsets for FAST (radius 3, 16 pixels) */
static const int circle_offsets[16][2] = {
    { 0, -3}, { 1, -3}, { 2, -2}, { 3, -1},
    { 3,  0}, { 3,  1}, { 2,  2}, { 1,  3},
    { 0,  3}, {-1,  3}, {-2,  2}, {-3,  1},
    {-3,  0}, {-3, -1}, {-2, -2}, {-1, -3}
};

/* Default FAST threshold */
#define DEFAULT_THRESHOLD 20
#define MIN_CONTIGUOUS 9
#define INITIAL_CAPACITY 1024

/**
 * @brief Check if pixel is a FAST corner
 */
static int is_corner(const uint8_t* data, int width, int height, 
                     int x, int y, int threshold) {
    if (x < 3 || y < 3 || x >= width - 3 || y >= height - 3) {
        return 0;
    }

    int center = data[y * width + x];
    int bright_count = 0;
    int dark_count = 0;
    int max_contiguous_bright = 0;
    int max_contiguous_dark = 0;
    int current_bright = 0;
    int current_dark = 0;

    /* Check all 16 pixels on the circle */
    for (int i = 0; i < 32; i++) {  /* Loop twice for wraparound */
        int idx = i % 16;
        int px = x + circle_offsets[idx][0];
        int py = y + circle_offsets[idx][1];
        int pixel = data[py * width + px];

        if (pixel > center + threshold) {
            current_bright++;
            current_dark = 0;
            if (current_bright > max_contiguous_bright) {
                max_contiguous_bright = current_bright;
            }
        } else if (pixel < center - threshold) {
            current_dark++;
            current_bright = 0;
            if (current_dark > max_contiguous_dark) {
                max_contiguous_dark = current_dark;
            }
        } else {
            current_bright = 0;
            current_dark = 0;
        }

        if (i < 16) {
            if (pixel > center + threshold) bright_count++;
            if (pixel < center - threshold) dark_count++;
        }
    }

    return (max_contiguous_bright >= MIN_CONTIGUOUS || 
            max_contiguous_dark >= MIN_CONTIGUOUS);
}

/**
 * @brief Compute corner response (sum of absolute differences)
 */
static float compute_corner_response(const uint8_t* data, int width, 
                                     int x, int y) {
    int center = data[y * width + x];
    float response = 0.0f;

    for (int i = 0; i < 16; i++) {
        int px = x + circle_offsets[i][0];
        int py = y + circle_offsets[i][1];
        int diff = data[py * width + px] - center;
        response += (float)(diff * diff);
    }

    return response;
}

Corner* cv_fast_corners(Image* input, int* num_corners) {
    return cv_fast_corners_ex(input, DEFAULT_THRESHOLD, 1, num_corners);
}

Corner* cv_fast_corners_ex(Image* input, int threshold, 
                           int non_max_suppression, int* num_corners) {
    if (!input || !input->data || !num_corners) {
        fprintf(stderr, "[CV] Error: Invalid input parameters\n");
        *num_corners = 0;
        return NULL;
    }

    printf("[CV] FAST corner detection: %ux%u, threshold=%d\n", 
           input->width, input->height, threshold);

    /* Initial allocation */
    int capacity = INITIAL_CAPACITY;
    Corner* corners = (Corner*)malloc(capacity * sizeof(Corner));
    if (!corners) {
        fprintf(stderr, "[CV] Error: Memory allocation failed\n");
        *num_corners = 0;
        return NULL;
    }

    int count = 0;
    int width = (int)input->width;
    int height = (int)input->height;

    /* Detect corners */
    for (int y = 3; y < height - 3; y++) {
        for (int x = 3; x < width - 3; x++) {
            if (is_corner(input->data, width, height, x, y, threshold)) {
                /* Grow array if needed */
                if (count >= capacity) {
                    capacity *= 2;
                    Corner* new_corners = (Corner*)realloc(corners, 
                                                    capacity * sizeof(Corner));
                    if (!new_corners) {
                        free(corners);
                        *num_corners = 0;
                        return NULL;
                    }
                    corners = new_corners;
                }

                corners[count].x = (float)x;
                corners[count].y = (float)y;
                corners[count].strength = compute_corner_response(
                    input->data, width, x, y);
                count++;
            }
        }
    }

    /* TODO: Implement non-maximum suppression */
    if (non_max_suppression && count > 0) {
        /* Placeholder - full NMS would filter overlapping corners */
    }

    *num_corners = count;
    printf("[CV] Detected %d corners\n", count);
    
    return corners;
}
