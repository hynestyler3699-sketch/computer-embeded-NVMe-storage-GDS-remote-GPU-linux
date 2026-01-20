/**
 * @file image_processing.c
 * @brief Basic image processing operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cv_algorithms.h"

Image* cv_image_create(uint32_t width, uint32_t height) {
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) {
        fprintf(stderr, "[CV] Error: Failed to allocate Image struct\n");
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->data = (uint8_t*)calloc(width * height, sizeof(uint8_t));
    
    if (!img->data) {
        fprintf(stderr, "[CV] Error: Failed to allocate image data\n");
        free(img);
        return NULL;
    }

    return img;
}

void cv_image_free(Image* img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

Image* cv_image_load_raw(const char* path, uint32_t width, uint32_t height) {
    if (!path) {
        fprintf(stderr, "[CV] Error: Invalid path\n");
        return NULL;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "[CV] Error: Cannot open file: %s\n", path);
        return NULL;
    }

    Image* img = cv_image_create(width, height);
    if (!img) {
        fclose(fp);
        return NULL;
    }

    size_t expected = (size_t)width * height;
    size_t read = fread(img->data, 1, expected, fp);
    fclose(fp);

    if (read != expected) {
        fprintf(stderr, "[CV] Warning: Read %zu bytes, expected %zu\n", 
                read, expected);
    }

    printf("[CV] Loaded image: %s (%ux%u)\n", path, width, height);
    return img;
}

int cv_image_save_raw(const Image* img, const char* path) {
    if (!img || !img->data || !path) {
        fprintf(stderr, "[CV] Error: Invalid parameters\n");
        return -1;
    }

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "[CV] Error: Cannot create file: %s\n", path);
        return -1;
    }

    size_t size = (size_t)img->width * img->height;
    size_t written = fwrite(img->data, 1, size, fp);
    fclose(fp);

    if (written != size) {
        fprintf(stderr, "[CV] Error: Write incomplete\n");
        return -1;
    }

    printf("[CV] Saved image: %s (%ux%u)\n", path, img->width, img->height);
    return 0;
}

void cv_normalize_image(Image* img, float min_val, float max_val) {
    if (!img || !img->data) {
        fprintf(stderr, "[CV] Error: Invalid image\n");
        return;
    }

    size_t size = (size_t)img->width * img->height;
    
    /* Find current min/max */
    uint8_t curr_min = 255;
    uint8_t curr_max = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (img->data[i] < curr_min) curr_min = img->data[i];
        if (img->data[i] > curr_max) curr_max = img->data[i];
    }

    /* Avoid division by zero */
    if (curr_max == curr_min) {
        memset(img->data, (int)min_val, size);
        return;
    }

    /* Normalize */
    float scale = (max_val - min_val) / (float)(curr_max - curr_min);
    
    for (size_t i = 0; i < size; i++) {
        float val = (img->data[i] - curr_min) * scale + min_val;
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        img->data[i] = (uint8_t)val;
    }

    printf("[CV] Normalized image to [%.1f, %.1f]\n", min_val, max_val);
}

Image* cv_gaussian_blur(Image* input, int kernel_size, float sigma) {
    if (!input || !input->data) {
        return NULL;
    }

    if (kernel_size % 2 == 0) {
        fprintf(stderr, "[CV] Error: Kernel size must be odd\n");
        return NULL;
    }

    Image* output = cv_image_create(input->width, input->height);
    if (!output) {
        return NULL;
    }

    /* Generate Gaussian kernel */
    int half = kernel_size / 2;
    float* kernel = (float*)malloc(kernel_size * kernel_size * sizeof(float));
    if (!kernel) {
        cv_image_free(output);
        return NULL;
    }

    float sum = 0.0f;
    for (int y = -half; y <= half; y++) {
        for (int x = -half; x <= half; x++) {
            float val = expf(-(x*x + y*y) / (2.0f * sigma * sigma));
            kernel[(y + half) * kernel_size + (x + half)] = val;
            sum += val;
        }
    }

    /* Normalize kernel */
    for (int i = 0; i < kernel_size * kernel_size; i++) {
        kernel[i] /= sum;
    }

    /* Apply convolution */
    for (uint32_t y = 0; y < input->height; y++) {
        for (uint32_t x = 0; x < input->width; x++) {
            float result = 0.0f;

            for (int ky = -half; ky <= half; ky++) {
                for (int kx = -half; kx <= half; kx++) {
                    int px = (int)x + kx;
                    int py = (int)y + ky;

                    /* Clamp to image bounds */
                    if (px < 0) px = 0;
                    if (py < 0) py = 0;
                    if (px >= (int)input->width) px = input->width - 1;
                    if (py >= (int)input->height) py = input->height - 1;

                    float k = kernel[(ky + half) * kernel_size + (kx + half)];
                    result += input->data[py * input->width + px] * k;
                }
            }

            output->data[y * output->width + x] = (uint8_t)result;
        }
    }

    free(kernel);
    printf("[CV] Applied Gaussian blur (kernel=%d, sigma=%.2f)\n", 
           kernel_size, sigma);
    
    return output;
}

Image* cv_resize(Image* input, uint32_t new_width, uint32_t new_height) {
    if (!input || !input->data) {
        return NULL;
    }

    Image* output = cv_image_create(new_width, new_height);
    if (!output) {
        return NULL;
    }

    float x_ratio = (float)input->width / new_width;
    float y_ratio = (float)input->height / new_height;

    for (uint32_t y = 0; y < new_height; y++) {
        for (uint32_t x = 0; x < new_width; x++) {
            /* Bilinear interpolation */
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;
            
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            if (x1 >= (int)input->width) x1 = input->width - 1;
            if (y1 >= (int)input->height) y1 = input->height - 1;

            float x_diff = src_x - x0;
            float y_diff = src_y - y0;

            uint8_t p00 = input->data[y0 * input->width + x0];
            uint8_t p10 = input->data[y0 * input->width + x1];
            uint8_t p01 = input->data[y1 * input->width + x0];
            uint8_t p11 = input->data[y1 * input->width + x1];

            float val = p00 * (1 - x_diff) * (1 - y_diff) +
                       p10 * x_diff * (1 - y_diff) +
                       p01 * (1 - x_diff) * y_diff +
                       p11 * x_diff * y_diff;

            output->data[y * new_width + x] = (uint8_t)val;
        }
    }

    printf("[CV] Resized image: %ux%u -> %ux%u\n", 
           input->width, input->height, new_width, new_height);
    
    return output;
}
