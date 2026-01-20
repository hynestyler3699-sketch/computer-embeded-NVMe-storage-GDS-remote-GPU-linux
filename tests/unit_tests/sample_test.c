/**
 * @file sample_test.c
 * @brief Sample unit tests for CV algorithms
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/cv_algorithms.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  TEST: %s... ", #name); \
        tests_run++; \
        if (test_##name()) { \
            printf("PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("FAILED\n"); \
        } \
    } while(0)

/* ============================================================================
 * Test Functions
 * ============================================================================ */

int test_image_create_free(void) {
    Image* img = cv_image_create(100, 100);
    if (!img) return 0;
    if (img->width != 100 || img->height != 100) {
        cv_image_free(img);
        return 0;
    }
    if (!img->data) {
        cv_image_free(img);
        return 0;
    }
    cv_image_free(img);
    return 1;
}

int test_image_normalize(void) {
    Image* img = cv_image_create(10, 10);
    if (!img) return 0;
    
    /* Fill with values 0-99 */
    for (int i = 0; i < 100; i++) {
        img->data[i] = (uint8_t)i;
    }
    
    /* Normalize to 0-255 */
    cv_normalize_image(img, 0.0f, 255.0f);
    
    /* Check min and max */
    uint8_t min_val = 255, max_val = 0;
    for (int i = 0; i < 100; i++) {
        if (img->data[i] < min_val) min_val = img->data[i];
        if (img->data[i] > max_val) max_val = img->data[i];
    }
    
    cv_image_free(img);
    
    /* Min should be 0, max should be 255 */
    return (min_val == 0 && max_val == 255);
}

int test_sobel_edge_detection(void) {
    Image* img = cv_image_create(50, 50);
    if (!img) return 0;
    
    /* Create a vertical edge in the middle */
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 50; x++) {
            img->data[y * 50 + x] = (x < 25) ? 0 : 255;
        }
    }
    
    Image* edges = cv_sobel_edge_detection(img);
    if (!edges) {
        cv_image_free(img);
        return 0;
    }
    
    /* Check that edge is detected around x=25 */
    int edge_found = 0;
    for (int y = 5; y < 45; y++) {
        if (edges->data[y * 50 + 25] > 100) {
            edge_found = 1;
            break;
        }
    }
    
    cv_image_free(img);
    cv_image_free(edges);
    
    return edge_found;
}

int test_fast_corners_empty(void) {
    Image* img = cv_image_create(50, 50);
    if (!img) return 0;
    
    /* Blank image - should have no corners */
    memset(img->data, 128, 50 * 50);
    
    int num_corners = 0;
    Corner* corners = cv_fast_corners(img, &num_corners);
    
    cv_image_free(img);
    
    /* Should return empty or NULL */
    if (corners) free(corners);
    
    return (num_corners == 0);
}

int test_resize(void) {
    Image* img = cv_image_create(100, 100);
    if (!img) return 0;
    
    /* Fill with pattern */
    for (int i = 0; i < 100 * 100; i++) {
        img->data[i] = (uint8_t)(i % 256);
    }
    
    Image* resized = cv_resize(img, 50, 50);
    if (!resized) {
        cv_image_free(img);
        return 0;
    }
    
    int success = (resized->width == 50 && resized->height == 50);
    
    cv_image_free(img);
    cv_image_free(resized);
    
    return success;
}

int test_gaussian_blur(void) {
    Image* img = cv_image_create(50, 50);
    if (!img) return 0;
    
    /* Add a spike in the center */
    memset(img->data, 0, 50 * 50);
    img->data[25 * 50 + 25] = 255;
    
    Image* blurred = cv_gaussian_blur(img, 5, 1.0f);
    if (!blurred) {
        cv_image_free(img);
        return 0;
    }
    
    /* Center should be lower, neighbors should be higher */
    int center = blurred->data[25 * 50 + 25];
    int neighbor = blurred->data[25 * 50 + 26];
    
    int success = (center > 0 && center < 255 && neighbor > 0);
    
    cv_image_free(img);
    cv_image_free(blurred);
    
    return success;
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void) {
    printf("=== CV Algorithms Unit Tests ===\n\n");
    
    printf("Image Operations:\n");
    TEST(image_create_free);
    TEST(image_normalize);
    TEST(resize);
    TEST(gaussian_blur);
    
    printf("\nEdge Detection:\n");
    TEST(sobel_edge_detection);
    
    printf("\nFeature Detection:\n");
    TEST(fast_corners_empty);
    
    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);
    
    return (tests_passed == tests_run) ? 0 : 1;
}
